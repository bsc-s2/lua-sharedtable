#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/**
 * slab.c
 *
 * slab pool for lua shared table.
 */
#include "slab.h"


#define is_huge_obj_group(group) ((group)->obj_size == ST_SLAB_HUGE_OBJ_SIZE)
#define is_huge_obj_pages(master) \
    ((master)->slab.obj_size > (1 << ST_SLAB_OBJ_SIZE_MAX_SHIFT))


/**
 *   size:    2^n    ,    2^n + 1,    ...  ,    2^(n+1) - 1
 *     --:    2^n - 1,    2^n    ,    2^n  ,    2^(n+1) - 2
 *    msb:    n      ,    n + 1  ,    n + 1,    n + 1
 *  index:    n      ,    n + 1  ,    n + 1,    n + 1
 */
int
st_slab_size_to_index(uint64_t size)
{

#if ULONG_MAX != UINT64_MAX
#error "unsigned long bit width is not the same with uint64_t"
#endif

    st_must(size > 0, ST_ARG_INVALID);

    if (size <= (1 << ST_SLAB_OBJ_SIZE_MIN_SHIFT)) {
        return ST_SLAB_OBJ_SIZE_MIN_SHIFT;
    }

    if (size > (1 << ST_SLAB_OBJ_SIZE_MAX_SHIFT)) {
        return ST_SLAB_GROUP_CNT - 1;
    }

    size--;

    return st_bit_msb(size) + 1;
}

static int
st_slab_group_init(st_slab_group_t *slab_group,
                   ssize_t obj_size,
                   st_pagepool_t *page_pool)
{
    st_must(slab_group != NULL, ST_ARG_INVALID);
    st_must(slab_group->inited == 0, ST_INITTWICE);
    st_must(page_pool != NULL, ST_ARG_INVALID);

    slab_group->obj_size  = obj_size;
    slab_group->page_pool = page_pool;

    int ret;
    if (gettimeofday(&slab_group->start_time, NULL) != 0) {
        ret = errno;
        derrno("failed to init start_time");

        return ret;
    }

    st_list_init(&slab_group->slabs_full);
    st_list_init(&slab_group->slabs_partial);

    ret = st_robustlock_init(&slab_group->lock);
    if (ret != ST_OK) {
        derr("failed to init group lock");

        return ret;
    }

    slab_group->inited = 1;

    return ST_OK;
}

int
st_slab_pool_init(st_slab_pool_t *slab_pool)
{
    st_must(slab_pool != NULL, ST_ARG_INVALID);

    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        int ret = st_slab_group_init(&slab_pool->groups[cnt],
                                     (1 << cnt),
                                     &slab_pool->page_pool);

        if (ret != ST_OK && ret != ST_INITTWICE) {
            return ret;
        }
    }

    return ST_OK;
}

/**
 * master pages would be released before upper module destroy slab.
 */
static int
st_slab_group_destroy(st_slab_group_t *slab_group)
{
    st_must(slab_group != NULL, ST_ARG_INVALID);

    if (!st_list_empty(&slab_group->slabs_full)) {
        derr("slabs_full is not empty");

        return ST_STATE_INVALID;
    }

    if (!st_list_empty(&slab_group->slabs_partial)) {
        derr("slabs_partial is not empty");

        return ST_STATE_INVALID;
    }

    int ret = ST_OK;
    if (slab_group->inited) {
        slab_group->page_pool = NULL;

        ret = st_robustlock_destroy(&slab_group->lock);

        if (ret == ST_OK) {
            slab_group->inited = 0;
        }
    }

    return ret;
}

int
st_slab_pool_destroy(st_slab_pool_t *slab_pool)
{
    st_must(slab_pool != NULL, ST_ARG_INVALID);

    int ret = ST_OK;

    for (int idx = ST_SLAB_OBJ_SIZE_MIN_SHIFT; idx < ST_SLAB_GROUP_CNT; idx++) {
        int result = st_slab_group_destroy(&slab_pool->groups[idx]);

        if (result != ST_OK) {
            ret = ret == ST_OK ? result : ret;
        }
    }

    return ret;
}

void
st_slab_update_obj_stat(st_slab_group_t *group, int alloc_cnt, int free_cnt)
{
    ssize_t alloc_diff = alloc_cnt - free_cnt;

    group->stat.current.free.cnt -= alloc_diff;
    group->stat.current.alloc.cnt += alloc_diff;

    /** no overflow check since ssize_t is really huge 9223372036854775807 */
    group->stat.obj.alloc.times += alloc_cnt;
    group->stat.obj.free.times += free_cnt;
}

void
st_slab_update_pages_stat(st_slab_group_t *group, int alloc_cnt, int free_cnt)
{
    ssize_t cnt = ST_SLAB_OBJ_CNT_EACH_ALLOC;
    if (is_huge_obj_group(group)) {
        cnt = 1;
    }

    ssize_t alloc_diff = (alloc_cnt - free_cnt) * cnt;

    group->stat.current.free.cnt += alloc_diff;

    /** no overflow check since ssize_t is really huge 9223372036854775807 */
    group->stat.pages.alloc.times += alloc_cnt;
    group->stat.pages.free.times += free_cnt;
}

static void
st_slab_set_pages_group_master(st_pagepool_page_t *master,
                               st_slab_group_t *group)
{
    master->slab.group = group;

    for (int cnt = 1; cnt < master->compound_page_cnt; cnt++) {
        master[cnt].slab.master = master;
    }
}

static void
st_slab_clear_pages_group_master(st_pagepool_page_t *master)
{
    master->slab.group = NULL;

    for (int cnt = 1; cnt < master->compound_page_cnt; cnt++) {
        master[cnt].slab.master = NULL;
    }
}

static int
st_slab_alloc_pages(st_slab_group_t *group, ssize_t obj_size, ssize_t obj_cnt)
{
    st_pagepool_page_t *pages = NULL;

    ssize_t page_size = group->page_pool->page_size;
    ssize_t pages_cnt = st_align(obj_size * obj_cnt, page_size) / page_size;

    int ret = st_pagepool_alloc_pages(group->page_pool, pages_cnt, &pages);
    if (ret != ST_OK) {
        return ret;
    }

    memset(pages->slab.bitmap, 0, sizeof(pages->slab.bitmap));
    pages->slab.obj_size = obj_size;

    st_slab_set_pages_group_master(pages, group);

    st_list_insert_last(&group->slabs_partial, &pages->lnode);

    st_slab_update_pages_stat(group, 1, 0);

    return ST_OK;
}

static inline int
st_slab_is_full(st_pagepool_page_t *master)
{
    int nbits = ST_SLAB_OBJ_CNT_EACH_ALLOC;
    if (is_huge_obj_pages(master)) {
        nbits = 1;
    }

    return st_bitmap_are_all_set(master->slab.bitmap, nbits);
}

static inline int
st_slab_is_all_free(st_pagepool_page_t *master)
{
    int nbits = ST_SLAB_OBJ_CNT_EACH_ALLOC;
    if (is_huge_obj_pages(master)) {
        nbits = 1;
    }

    return st_bitmap_are_all_cleared(master->slab.bitmap, nbits);
}

static int
st_slab_get_obj_from_group(st_slab_group_t *group, ssize_t obj_size, void **obj)
{
    int ret     = ST_OK;
    int obj_cnt = ST_SLAB_OBJ_CNT_EACH_ALLOC;

    if (is_huge_obj_group(group)) {
        /** alloc only one object for extra size group */
        obj_cnt = 1;

    } else {
        /** normal slab use group fixed-size */
        obj_size = group->obj_size;

    }

    ret = st_robustlock_lock(&group->lock);
    if (ret != 0) {
        derrno("failed to lock group");

        return ret;
    }

    if (st_list_empty(&group->slabs_partial)) {
        ret = st_slab_alloc_pages(group, obj_size, obj_cnt);
        if (ret != ST_OK) {
            goto quit;
        }
    }

    uint8_t *base              = NULL;
    st_list_t *node            = st_list_last(&group->slabs_partial);
    st_pagepool_page_t *master = st_owner(node, st_pagepool_page_t, lnode);

    ret = st_pagepool_page_to_addr(group->page_pool, master, &base);
    if (ret != ST_OK) {
        goto quit;
    }

    int idx = st_bitmap_find_clear_bit(master->slab.bitmap, 0, obj_cnt);
    st_assert(idx >= 0);

    /** alignment can be add here if needed */
    *obj = (void *)((uintptr_t)base + idx * obj_size);

    st_bitmap_set(master->slab.bitmap, idx);

    /** update statistics info */
    st_slab_update_obj_stat(group, 1, 0);

    if (st_slab_is_full(master)) {
        /** move pages from slabs_partial to slabs_full */
        st_list_remove(&master->lnode);
        st_list_insert_last(&group->slabs_full, &master->lnode);
    }

quit:
    st_robustlock_unlock_err_abort(&group->lock);
    return ret;
}

int
st_slab_obj_alloc(st_slab_pool_t *slab_pool, ssize_t size, void **ret_addr)
{
    st_must(slab_pool != NULL, ST_ARG_INVALID);
    st_must(size > 0, ST_ARG_INVALID);
    st_must(ret_addr != NULL, ST_ARG_INVALID);

    int index = st_slab_size_to_index((uint64_t)size);
    return st_slab_get_obj_from_group(&slab_pool->groups[index],
                                      size,
                                      ret_addr);
}

st_pagepool_page_t *
st_slab_get_master_page(st_pagepool_page_t *page)
{
    st_must(page != NULL, NULL);

    st_pagepool_page_t *master = page;
    if (page->type != ST_PAGEPOOL_PAGE_MASTER) {
        master = (st_pagepool_page_t *)page->slab.master;
    }

    if (master == NULL || master->slab.group == NULL) {
        return NULL;
    }

    return master;
}

static int
st_slab_clear_bitmap_by_addr(st_slab_group_t *group,
                             st_pagepool_page_t *master,
                             void *addr)
{
    uint8_t *base = NULL;
    int ret = st_pagepool_page_to_addr(group->page_pool, master, &base);
    if (ret != ST_OK) {
        return ret;
    }

    uintptr_t offset = (uintptr_t)addr - (uintptr_t)base;
    if (offset % group->obj_size != 0) {
        return ST_ARG_INVALID;
    }

    int index = offset / group->obj_size;
    if (index >= ST_SLAB_OBJ_CNT_EACH_ALLOC) {
        return ST_ARG_INVALID;
    }

    if (group->obj_size == ST_SLAB_HUGE_OBJ_SIZE && index != 0) {
        return ST_ARG_INVALID;
    }

    if (st_bitmap_clear(master->slab.bitmap, index) == 0) {
        return ST_AGAIN;
    }

    return ST_OK;
}

static int
st_slab_free_obj_from_group(st_slab_group_t *group,
                            st_pagepool_page_t *master,
                            void *addr)
{
    int ret = st_robustlock_lock(&group->lock);
    if (ret != ST_OK) {
        derr("failed to lock group");

        return ret;
    }

    /** clear according bitmap */
    int was_in_full = st_slab_is_full(master);

    ret = st_slab_clear_bitmap_by_addr(group, master, addr);
    if (ret != ST_OK) {
        goto quit;
    }

    st_slab_update_obj_stat(group, 0, 1);

    /** free pages if necessary */
    if (st_slab_is_all_free(master)) {
         /** remove node and return to page_pool */
        st_list_remove(&master->lnode);
        st_slab_clear_pages_group_master(master);

        ret = st_pagepool_free_pages(group->page_pool, master);
        if (ret != ST_OK) {
            derr("failed to free slabs pages %d", ret);

            st_slab_set_pages_group_master(master, group);
            st_list_insert_last(&group->slabs_partial, &master->lnode);

            goto quit;
        }

        st_slab_update_pages_stat(group, 0, 1);
    }
    else if (was_in_full) {
        st_list_remove(&master->lnode);
        st_list_insert_last(&group->slabs_partial, &master->lnode);
    }

quit:
    st_robustlock_unlock_err_abort(&group->lock);
    return ret;
}

int
st_slab_obj_free(st_slab_pool_t *slab_pool, void *addr)
{
    st_must(slab_pool != NULL, ST_ARG_INVALID);
    st_must(addr != NULL, ST_ARG_INVALID);

    st_pagepool_page_t *page = NULL;

    ssize_t page_size = slab_pool->page_pool.page_size;
    int ret = st_pagepool_addr_to_page(&slab_pool->page_pool,
                                       ADDR_PAGE_BASE(addr, page_size),
                                       &page);
    if (ret != ST_OK) {
        return ret;
    }

    st_pagepool_page_t *master = st_slab_get_master_page(page);
    /** not slab page */
    st_must(master != NULL, ST_OUT_OF_RANGE);

    int index = st_slab_size_to_index(master->slab.obj_size);
    st_slab_group_t *group = &slab_pool->groups[index];

    st_assert(master->slab.group == group);

    return st_slab_free_obj_from_group(group, master, addr);
}
