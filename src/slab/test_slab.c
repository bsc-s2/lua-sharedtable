#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "slab.h"
#include "unittest/unittest.h"


typedef struct {
    void *base;
    int  shm_fd;
    void *data;
    st_slab_pool_t *slab_pool;
} setup_info_t;


#define ST_TEST_SLAB_HUGE_OBJ_SIZE       (st_page_size() + 1)
#define ST_TEST_SLAB_REGION_CNT          10
#define ST_TEST_SLAB_SHARED_SPACE_LENGTH (1024 * 1024 * 200)

#define CHECK_GROUP_FIELD(index, group, field, value) do {                 \
    st_ut_eq(value, group->field, "[%d] failed to update " #field, index); \
} while(0)

#define CHECK_GROUP_STATISTICS(index, group, values) do {                    \
    CHECK_GROUP_FIELD(index, group, stat.current.free.cnt, (values)[0]);     \
    CHECK_GROUP_FIELD(index, group, stat.current.alloc.cnt, (values)[1]);    \
    CHECK_GROUP_FIELD(index, group, stat.obj.alloc.times, (values)[2]);      \
    CHECK_GROUP_FIELD(index, group, stat.obj.free.times, (values)[3]);       \
    CHECK_GROUP_FIELD(index, group, stat.pages.alloc.times, (values)[4]);    \
    CHECK_GROUP_FIELD(index, group, stat.pages.free.times, (values)[5]);     \
} while(0)


static void
st_slab_setup(setup_info_t *info)
{
    int ret = st_region_shm_create(ST_TEST_SLAB_SHARED_SPACE_LENGTH,
                                   &info->base,
                                   &info->shm_fd);
    st_assert(ret == ST_OK);

    ssize_t page_size = st_page_size();
    info->slab_pool = (st_slab_pool_t *)info->base;
    info->data = (void *)(st_align((uintptr_t)info->base + sizeof(*info->slab_pool),
                                   page_size));

    ssize_t cfg_len = (uintptr_t)info->data - (uintptr_t)info->base;
    ssize_t data_len = ST_TEST_SLAB_SHARED_SPACE_LENGTH - cfg_len;

    ret = st_region_init(&info->slab_pool->page_pool.region_cb,
                         info->data,
                         data_len / page_size / ST_TEST_SLAB_REGION_CNT,
                         ST_TEST_SLAB_REGION_CNT,
                         1);
    st_assert(ret == ST_OK);

    ret = st_pagepool_init(&info->slab_pool->page_pool, page_size);
    st_assert(ret == ST_OK);

    ret = st_slab_pool_init(info->slab_pool);
    st_assert(ret == ST_OK);
}

static void
st_slab_cleanup(setup_info_t *info, int must_ok)
{
    int ret;

    ret = st_region_destroy(&info->slab_pool->page_pool.region_cb);
    st_assert(ret == ST_OK);

    ret = st_pagepool_destroy(&info->slab_pool->page_pool);
    st_assert(ret == ST_OK);

    ret = st_slab_pool_destroy(info->slab_pool);
    if (must_ok) {
        st_assert(ret == ST_OK);
    }

    ret = st_region_shm_destroy(info->shm_fd,
                                info->base,
                                ST_TEST_SLAB_SHARED_SPACE_LENGTH);
    st_assert(ret == ST_OK);
}

extern int st_slab_size_to_index(uint32_t size);

st_test(st_slab, size_to_index)
{
    struct size_to_index {
        uint32_t size;
        int      index;
    } cases [] = {
        {0, ST_ARG_INVALID},
        {1, 3},
        {7, 3},
        {8, 3},
        {9, 4},
        {15, 4},
        {16, 4},
        {17, 5},
        {31, 5},
        {32, 5},
        {33, 6},
        {63, 6},
        {64, 6},
        {65, 7},
        {127, 7},
        {128, 7},
        {129, 8},
        {255, 8},
        {256, 8},
        {257, 9},
        {511, 9},
        {512, 9},
        {513, 10},
        {1023, 10},
        {1024, 10},
        {1025, 11},
        {2047, 11},
        {2048, 11},
        {2049, 12},
        {3000, 12},
        {4096, 12},
    };

    for (int cnt = 0; cnt < st_nelts(cases); cnt++) {
        int rst = st_slab_size_to_index(cases[cnt].size);
        st_ut_eq(cases[cnt].index, rst, "failed to calc index %d", cnt);
    }
}

void st_slab_update_obj_stat(st_slab_group_t *group, int alloc, int free);
void st_slab_update_pages_stat(st_slab_group_t *group, int alloc, int free);

st_test(st_slab, update_statistics)
{
    st_slab_pool_t slab_pool;
    memset(&slab_pool, 0, sizeof(slab_pool));
    st_ut_eq(ST_OK, st_slab_pool_init(&slab_pool), "failed to init slab");

    /** normal slab group */
    st_slab_group_t *group = &slab_pool.groups[0];

    /** alloc pages */
    st_slab_update_pages_stat(group, 1, 0);
    ssize_t *values = (ssize_t []) {
        ST_SLAB_OBJ_CNT_EACH_ALLOC,
        0,
        0,
        0,
        1,
        0,
    };
    CHECK_GROUP_STATISTICS(1, group, values);

    /** alloc obj */
    st_slab_update_obj_stat(group, 1, 0);
    values = (ssize_t []) {
        ST_SLAB_OBJ_CNT_EACH_ALLOC - 1,
        1,
        1,
        0,
        1,
        0,
    };
    CHECK_GROUP_STATISTICS(2, group, values);

    /** free obj */
    st_slab_update_obj_stat(group, 0, 1);
    values = (ssize_t []) {
        ST_SLAB_OBJ_CNT_EACH_ALLOC,
        0,
        1,
        1,
        1,
        0,
    };
    CHECK_GROUP_STATISTICS(3, group, values);

    /** free pages */
    st_slab_update_pages_stat(group, 0, 1);
    values = (ssize_t []) {
        0,
        0,
        1,
        1,
        1,
        1,
    };
    CHECK_GROUP_STATISTICS(4, group, values);

    /** extra size group */
    group = &slab_pool.groups[ST_SLAB_GROUP_CNT-1];
    st_slab_update_pages_stat(group, 2, 0);
    values = (ssize_t []) {
        2,
        0,
        0,
        0,
        2,
        0,
    };
    CHECK_GROUP_STATISTICS(5, group, values);

    st_slab_update_pages_stat(group, 0, 1);
    values = (ssize_t []) {
        1,
        0,
        0,
        0,
        2,
        1,
    };
    CHECK_GROUP_STATISTICS(6, group, values);
}

static void
st_test_check_unused_groups(setup_info_t *info)
{
    for (int gcnt = 0; gcnt < ST_SLAB_OBJ_SIZE_MIN_SHIFT; gcnt++) {
        st_slab_group_t *group = &info->slab_pool->groups[gcnt];

        st_ut_eq(0, group->obj_size, "failed to set obj_size");

        ssize_t values[] = {
            0, 0, 0, 0, 0, 0
        };
        CHECK_GROUP_STATISTICS(gcnt, group, values);

        st_ut_eq(0,
                 group->start_time.tv_sec,
                 "failed to set start_time.tv_sec");
        st_ut_eq(0,
                 group->start_time.tv_usec,
                 "failed to set start_time.tv_usec");

        st_ut_eq(NULL, group->slabs_partial.prev, "prev is not NULL");
        st_ut_eq(1,
                (group->slabs_partial.next == group->slabs_partial.prev) &&
                (group->slabs_full.prev == group->slabs_partial.prev) &&
                (group->slabs_full.next == group->slabs_full.prev),
                 "prev is not NULL");
    }
}

int st_slab_alloc_pages(st_slab_pool_t *slab_pool,
                        st_slab_group_t *group,
                        ssize_t pages_cnt);

st_test(st_slab, pool_init_destroy)
{
    st_slab_pool_t slab_pool;
    memset(&slab_pool, 0, sizeof(slab_pool));

    st_ut_eq(ST_ARG_INVALID, st_slab_pool_init(NULL), "NULL should fail");

    st_ut_eq(ST_OK, st_slab_pool_init(&slab_pool), "failed to init slab");
    st_ut_eq(ST_OK, st_slab_pool_init(&slab_pool), "failed to double init slab");
    st_ut_eq(ST_OK, st_slab_pool_init(&slab_pool), "failed to treble init slab");

    ssize_t obj_size = 8;
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        st_slab_group_t *group = &slab_pool.groups[cnt];

        st_ut_eq(group->obj_size, obj_size, "obj_size failed %d", cnt);
        ssize_t values[6] = { 0, 0, 0, 0, 0, 0 };
        CHECK_GROUP_STATISTICS(1, group, values);

        st_ut_ne(0, group->start_time.tv_sec, "tv_sec failed %d", cnt);
        st_ut_ne(0, group->start_time.tv_usec, "tv_usec failed %d", cnt);

        st_ut_eq(1,
                 st_list_empty(&group->slabs_full),
                 "slabs_full failed %d", cnt);
        st_ut_eq(1,
                 st_list_empty(&group->slabs_partial),
                 "slabs_partial failed %d", cnt);

        st_ut_eq(ST_OK,
                 st_robustlock_lock(&group->lock),
                 "lock lock failed %d", cnt);
        st_ut_eq(ST_OK,
                 st_robustlock_unlock(&group->lock),
                 "lock unlock failed %d", cnt);

        st_ut_eq(1, group->inited, "failed to set inited %d", cnt);

        st_ut_eq(&slab_pool.page_pool,
                 group->page_pool,
                 "failed to set page_pool %d", cnt);
        obj_size <<= 1;
    }

    st_ut_eq(ST_OK,
             st_slab_pool_destroy(&slab_pool),
             "failed to destroy slab");
    st_ut_eq(ST_OK,
             st_slab_pool_destroy(&slab_pool),
             "failed to double destroy slab");
    st_ut_eq(ST_OK,
             st_slab_pool_destroy(&slab_pool),
             "failed to treble destroy slab");

    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        st_slab_group_t *group = &slab_pool.groups[cnt];
        st_ut_eq(0, group->inited, "failed to set inited %d", cnt);
        st_ut_eq(NULL, group->page_pool, "failed to clear page_pool, %d", cnt);
    }
}

static void
st_test_slab_alloc_first_obj_each_group(setup_info_t *info, void **objs)
{
    /** alloc one obj in each group */
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        ssize_t size = 1 << cnt;

        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            size = ST_TEST_SLAB_HUGE_OBJ_SIZE;
        }

        int ret = st_slab_obj_alloc(info->slab_pool, size, &objs[cnt]);
        st_ut_eq(ST_OK, ret, "failed to alloc obj");

        st_slab_group_t *group = &info->slab_pool->groups[cnt];

        /** check obj_size */
        int obj_size = size;
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            obj_size = ST_SLAB_HUGE_OBJ_SIZE;
        }
        st_ut_eq(obj_size, group->obj_size, "failed to set obj_size");

        /** check group statistics */
        ssize_t *values = (ssize_t []) {
            ST_SLAB_OBJ_CNT_EACH_ALLOC - 1,
            1,
            1,
            0,
            1,
            0,
        };
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            values[0] = 0;
            values[1] = 1;
        }
        CHECK_GROUP_STATISTICS(1, group, values);

        /** check lists */
        st_list_t *node = &group->slabs_partial;
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            node = &group->slabs_full;
        }

        int nbits = ST_SLAB_OBJ_CNT_EACH_ALLOC;
        st_ut_eq(node,
                 node->next->next,
                 "obj alloc to wrong group %d", cnt);

        st_pagepool_page_t *master = st_owner(node->next,
                                              st_pagepool_page_t,
                                              lnode);
        st_ut_eq(0,
                 st_bitmap_find_set_bit(master->slab.bitmap, 0, 2),
                 "failed to set bitmap %d", cnt);
        st_ut_eq(-1,
                 st_bitmap_find_set_bit(master->slab.bitmap, 1, nbits),
                 "falsely set wrong bitmap");

        size = group->obj_size;
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            size = ST_TEST_SLAB_HUGE_OBJ_SIZE;
        }
        st_ut_eq(size,
                 master->slab.obj_size,
                 "failed to set obj_size");

        st_ut_eq((uintptr_t)group,
                 (uintptr_t)master->slab.group,
                 "failed to set master slab.group");

        /** check slave pages */
        for (int pcnt = 1; pcnt < master->compound_page_cnt; pcnt++) {
            st_ut_eq((uintptr_t)master,
                     (uintptr_t)master[pcnt].slab.master,
                     "failed to set slave slab.master");
        }
    }

    st_test_check_unused_groups(info);
}

static uint8_t
st_test_slab_get_random_byte(void)
{
    uint8_t byte;
    int fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1) {
        return 0xff;
    }

    if (1 != read(fd, &byte, 1)) {
        byte = 0xff;
    }

    close(fd);

    return 0xff;
}

static void
st_test_slab_write_value_and_check(setup_info_t *info, void **objs)
{
    uint8_t random_byte = st_test_slab_get_random_byte();

    for (int cnt = 0; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        ssize_t in_bytes = info->slab_pool->groups[cnt].obj_size;
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            in_bytes = ST_TEST_SLAB_HUGE_OBJ_SIZE;
        }

        /**
         * if overwriting anything,
         * hopefully, the following tests would crash.
         */
        memset(objs[cnt], random_byte, in_bytes);
    }
}

/**
 * each group has one object already.
 * use up the rest slab space and alloc one more obj to acquire another new
 * page for each group.
 */
static void
st_test_slab_alloc_the_second_pages(setup_info_t *info,
                                    const void *objs[],
                                    void *normal_addr[],
                                    void *extra_addr[])
{
    for (int gcnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; gcnt < ST_SLAB_GROUP_CNT; gcnt++) {
        /** test alloc new pages again */
        int ret                = ST_OK;
        void *addr             = (void *)objs[gcnt];
        st_slab_group_t *group = &info->slab_pool->groups[gcnt];
        ssize_t size           = group->obj_size;

        /** alloc to make the first pages to be full */
        for (int cnt = 1; cnt < ST_SLAB_OBJ_CNT_EACH_ALLOC; cnt++) {
            void *ret_addr = NULL;
            if (gcnt == ST_SLAB_GROUP_CNT - 1) {
                size = ST_TEST_SLAB_HUGE_OBJ_SIZE;
            }

            ret = st_slab_obj_alloc(info->slab_pool, size, &ret_addr);
            st_ut_eq(ST_OK, ret, "failed to alloc obj");

            if (gcnt != ST_SLAB_GROUP_CNT - 1) {
                st_ut_eq(size,
                         (uintptr_t)ret_addr - (uintptr_t)addr,
                         "obj addr alignment failed");

            } else {
                extra_addr[cnt - 1] = ret_addr;
                st_pagepool_page_t *master = st_owner(group->slabs_full.prev,
                                                      st_pagepool_page_t,
                                                      lnode);
                void *expected_addr = NULL;
                ret = st_pagepool_page_to_addr(&info->slab_pool->page_pool,
                                               master,
                                               (uint8_t **)&expected_addr);
                st_assert(ret == ST_OK);

                st_ut_eq(expected_addr, ret_addr, "addr not consistent");

                st_ut_eq(ST_TEST_SLAB_HUGE_OBJ_SIZE,
                         master->slab.obj_size,
                         "failed to set slab.obj_size");

                st_ut_eq(group, master->slab.group, "failed to set slab.group");
                st_ut_eq(master,
                        (master + 1)->slab.master,
                         "failed to set slave slab.master");
            }

            addr = ret_addr;
        }

        /** another one object to acquire another new pages */
        ret = st_slab_obj_alloc(info->slab_pool, size, &addr);
        st_ut_eq(ST_OK, ret, "failed to alloc obj when new pages needed");

        if (gcnt == ST_SLAB_GROUP_CNT - 1) {
            extra_addr[ST_SLAB_OBJ_CNT_EACH_ALLOC - 1] = addr;
            st_ut_eq(1,
                     st_list_empty(&group->slabs_partial),
                     "extra slabs_partial not empty");

        } else {
            normal_addr[gcnt] = addr;

            st_ut_eq(&group->slabs_full,
                     group->slabs_full.next->next,
                     "normal slabs_full has more than one master");
            st_ut_eq(&group->slabs_partial,
                     group->slabs_partial.next->next,
                     "normal slabs_partial has more than one master");

        }
    }

    st_test_check_unused_groups(info);

    /** check group statistics after all allocations are done */
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        st_slab_group_t *group = &info->slab_pool->groups[cnt];

        ssize_t *values = NULL;
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            values = (ssize_t []) {
                0,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                0,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                0,
            };

        } else {
            values = (ssize_t []) {
                ST_SLAB_OBJ_CNT_EACH_ALLOC - 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                0,
                2,
                0,
            };
        }

        CHECK_GROUP_STATISTICS(cnt, group, values);
    }
}

/**
 * test obj free
 *
 * every group has two masters, one in partial with one object allocated,
 * one in full with all objects allocated.
 * objs stores the address of the first obj in full pages in each group.
 *
 * extra group is special, there are two masters in full.
 */
static void
st_test_slab_free_first_obj_in_full_each_group(setup_info_t *info,
                                               const void *objs[])
{
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT; cnt++) {
        void *addr = (void *)objs[cnt];
        st_slab_group_t *group = &info->slab_pool->groups[cnt];

        st_list_t *node      = &group->slabs_full;
        st_list_t *to_remove = node->next;

        int ret = st_slab_obj_free(info->slab_pool, addr);
        st_ut_eq(ST_OK, ret, "failed to free obj");

        st_ut_ne(node->next, to_remove, "failed to remove first obj");

        if (cnt != ST_SLAB_GROUP_CNT - 1) {
            int nbits = ST_SLAB_OBJ_CNT_EACH_ALLOC;

            st_ut_eq(1,
                     st_list_empty(&group->slabs_full),
                     "slabs_full not empty");

            node = &group->slabs_partial;
            st_ut_eq(node,
                     node->next->next->next,
                     "not two master in slabs_full");

            ssize_t *values = (ssize_t []) {
                ST_SLAB_OBJ_CNT_EACH_ALLOC,
                ST_SLAB_OBJ_CNT_EACH_ALLOC,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                1,
                2,
                0,
            };
            CHECK_GROUP_STATISTICS(cnt, group, values);

            st_pagepool_page_t *master = st_owner(node->next->next,
                                                  st_pagepool_page_t,
                                                  lnode);

            st_ut_eq(group, master->slab.group, "wrong slab.group");
            st_ut_eq(0,
                     st_bitmap_find_clear_bit(master->slab.bitmap, 0, nbits),
                     "failed to clear bitmap");
            st_ut_eq(-1,
                     st_bitmap_find_clear_bit(master->slab.bitmap, 1, nbits),
                     "falsely clear bitmap");
        }
    }
}

extern st_pagepool_page_t *st_slab_get_master_page(st_pagepool_page_t *page);

static st_pagepool_page_t *
st_test_slab_get_master_from_addr(setup_info_t *info, void *addr)
{
    st_pagepool_page_t *page;

    int ret = st_pagepool_addr_to_page(&info->slab_pool->page_pool,
                                       ADDR_PAGE_BASE(addr, st_page_size()),
                                       &page);
    if (ret != ST_OK) {
        return NULL;
    }

    return st_slab_get_master_page(page);
}

static void
st_test_slab_free_rest_objs_in_first_pages(setup_info_t *info,
                                           const void *objs[])
{
    /** normal group */
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT - 1; cnt++) {
        st_pagepool_page_t *master;

        void *addr   = (void *)objs[cnt];
        st_slab_group_t *group = &info->slab_pool->groups[cnt];

        ssize_t size = group->obj_size;

        master = st_test_slab_get_master_from_addr(info, addr);
        int compound_page_cnt = master->compound_page_cnt;

        /** free the rest objs */
        for (int idx = 1; idx < ST_SLAB_OBJ_CNT_EACH_ALLOC; idx++) {
            int bit = st_bitmap_find_set_bit(master->slab.bitmap,
                                             0,
                                             ST_SLAB_OBJ_CNT_EACH_ALLOC);

            void *free_addr = (void *)((uintptr_t)addr + size);

            int ret = st_slab_obj_free(info->slab_pool, free_addr);
            st_ut_eq(ST_OK, ret, "failed to free obj %d: %d", cnt, idx);

            /**
             * the last free will give master pages back to page_pool,
             * and the memory space of bitmap is used by it for other purpuse,
             * so we can not check
             */
            if (idx != ST_SLAB_OBJ_CNT_EACH_ALLOC - 1) {
                st_ut_eq(0,
                         st_bitmap_get(master->slab.bitmap, bit),
                         "failed to clear bitmap");
            }

            addr = free_addr;
        }

        /**
         * clear slab.group of master and
         * slab.master of slave pages when free pages
         */
        st_ut_eq(NULL, master->slab.group, "failed to clear slab.group");
        for (int idx = 1; idx < compound_page_cnt; idx++) {
            st_ut_eq(NULL,
                     master[idx].slab.master,
                     "failed to clear slab.master %d", idx);
        }

        st_ut_eq(&group->slabs_partial,
                 group->slabs_partial.next->next,
                 "more than one master in slabs_partial");

        ssize_t values[] = {
            ST_SLAB_OBJ_CNT_EACH_ALLOC - 1,
            1,
            ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            ST_SLAB_OBJ_CNT_EACH_ALLOC,
            2,
            1,
        };

        CHECK_GROUP_STATISTICS(cnt, group, values);
    }
}

static void
st_test_slab_free_last_obj_in_normal_group(setup_info_t *info,
                                           const void *objs[])
{
    for (int cnt = ST_SLAB_OBJ_SIZE_MIN_SHIFT; cnt < ST_SLAB_GROUP_CNT - 1; cnt++) {
        int ret = st_slab_obj_free(info->slab_pool, (void *)objs[cnt]);
        st_ut_eq(ST_OK, ret, "failed to free obj");

        st_slab_group_t *group = &info->slab_pool->groups[cnt];

        st_ut_eq(1,
                 st_list_empty(&group->slabs_full),
                 "failed to clear slabs_full");
        st_ut_eq(1,
                 st_list_empty(&group->slabs_partial),
                 "failed to clear slabs_partial");

        ssize_t *values = (ssize_t []) {
            0,
            0,
            ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            2,
            2
        };
        if (cnt == ST_SLAB_GROUP_CNT - 1) {
            values = (ssize_t []) {
                0,
                0,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
                ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            };
        }

        CHECK_GROUP_STATISTICS(cnt, group, values);
    }
}

/**
 * the number of objects in extra group is ST_SLAB_OBJ_CNT_EACH_ALLOC,
 * and they are all in slabs_full.
 */
static void
st_test_slab_free_rest_objs_in_extra_group(setup_info_t *info, void *objs[])
{
    st_slab_pool_t *slab_pool = info->slab_pool;
    st_slab_group_t *group    = &slab_pool->groups[ST_SLAB_GROUP_CNT - 1];

    st_ut_eq(0,
             st_list_empty(&group->slabs_full),
             "slabs_full is not empty");
    st_ut_eq(1,
             st_list_empty(&group->slabs_partial),
             "slabs_partial is empty");

    for (int cnt = 0; cnt < ST_SLAB_OBJ_CNT_EACH_ALLOC; cnt++) {
        int ret       = ST_OK;
        uint8_t *addr = NULL;

        st_list_t *next_next = group->slabs_full.next->next;

        st_pagepool_page_t *master = st_owner(group->slabs_full.next,
                                              st_pagepool_page_t,
                                              lnode);
        int compound_page_cnt = master->compound_page_cnt;

        ret = st_pagepool_page_to_addr(&slab_pool->page_pool, master, &addr);
        st_assert(ret == ST_OK);
        st_ut_eq((void *)addr, objs[cnt], "address to caller is wrong");

        ret = st_slab_obj_free(slab_pool, objs[cnt]);
        st_ut_eq(ST_OK, ret, "failed to free obj");

        st_ut_eq(next_next, group->slabs_full.next, "failed to handle list");

        /** check slab.master and slab.group */
        st_pagepool_page_t *page = master;
        st_ut_eq(NULL, master->slab.group, "failed to clear slab.group");
        for (int idx = 1; idx < compound_page_cnt; idx++) {
            st_ut_eq(NULL,
                     master[idx].slab.master,
                     "failed to clear slab.master %d", idx);
            page++;
        }

        ssize_t values[] = {
            0,
            ST_SLAB_OBJ_CNT_EACH_ALLOC - cnt - 1,
            ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            2 + cnt,
            ST_SLAB_OBJ_CNT_EACH_ALLOC + 1,
            2 + cnt,
        };
        CHECK_GROUP_STATISTICS(cnt, group, values);
    }

    st_ut_eq(1,
             st_list_empty(&group->slabs_full),
             "slabs_full not empty");
    st_ut_eq(1,
             st_list_empty(&group->slabs_partial),
             "slabs_partial not empty");
}

st_test(st_slab, obj_alloc_free)
{
    setup_info_t info;
    st_slab_setup(&info);

    void *first_objs[ST_SLAB_GROUP_CNT];
    void *second_objs[ST_SLAB_GROUP_CNT];
    void *extra_objs[ST_SLAB_OBJ_CNT_EACH_ALLOC];

    /** alloc one obj in each group */
    st_test_slab_alloc_first_obj_each_group(&info, first_objs);

    /** write all bytes of objs */
    st_test_slab_write_value_and_check(&info, first_objs);

    /** alloc another pages from page_pool */
    st_test_slab_alloc_the_second_pages(&info,
                                        (const void **)first_objs,
                                        second_objs,
                                        extra_objs);

    st_test_slab_free_first_obj_in_full_each_group(&info,
                                                   (const void **)first_objs);

    /** free all objs in apges */
    st_test_slab_free_rest_objs_in_first_pages(&info,
                                               (const void **)first_objs);

    /** free all objs */
    st_test_slab_free_last_obj_in_normal_group(&info,
                                               (const void **)second_objs);

    st_test_slab_free_rest_objs_in_extra_group(&info, extra_objs);

    st_test_check_unused_groups(&info);

    st_slab_cleanup(&info, 1);
}

st_test(st_slab, obj_alloc_free_invalid)
{
    setup_info_t info;
    /** total memory space for data is 20 * 1024 * 4k = 80MB */
    st_slab_setup(&info);

    /** free NULL */
    void *addr = NULL;
    st_ut_eq(ST_ARG_INVALID,
             st_slab_obj_free(info.slab_pool, addr),
             "free NULL should be arg invalid error");

    /** free invalid addr not in region */
    addr = &addr;
    int ret = st_slab_obj_free(info.slab_pool, addr);
    st_ut_eq(ST_NOT_FOUND, ret, "should fail to free addr not in region");

    /** free invalid addr in region but not in page_pool */
    addr = info.base;
    ret = st_slab_obj_free(info.slab_pool, addr);
    st_ut_eq(ST_NOT_FOUND, ret, "should be not found error");

    /** double free obj in slab */
    void *objs[3];
    for (int cnt = 0; cnt < 3; cnt++) {
        ret = st_slab_obj_alloc(info.slab_pool, 12, &addr);
        st_ut_eq(ST_OK, ret, "failed to alloc obj");
        objs[cnt] = addr;
    }

    ret = st_slab_obj_free(info.slab_pool, objs[1]);
    st_ut_eq(ST_OK, ret, "failed to free obj the first time");

    ret = st_slab_obj_free(info.slab_pool, objs[1]);
    st_ut_eq(ST_AGAIN, ret, "should fail when double-free obj");

    /** free addr in page_pool but not in slab */
    st_pagepool_page_t *master = NULL;
    ret = st_pagepool_alloc_pages(&info.slab_pool->page_pool, 5, &master);
    st_ut_eq(ST_OK, ret, "failed to alloc pages");

    st_pagepool_page_t *page = master + 2;

    /** 1: addr not in data area */
    addr = (void *)page;
    ret = st_slab_obj_free(info.slab_pool, addr);
    st_ut_eq(ST_OUT_OF_RANGE, ret, "should fail for out-of-range");

    /** 2: addr in data area */
    ret = st_pagepool_page_to_addr(&info.slab_pool->page_pool,
                                   page,
                                   (uint8_t **)&addr);
    st_ut_eq(ST_OK, ret, "failed to page to addr");

    ret = st_slab_obj_free(info.slab_pool, addr);
    st_ut_eq(ST_OUT_OF_RANGE, ret, "should fail for out-of-range");

    st_pagepool_free_pages(&info.slab_pool->page_pool, master);

    /** out of memory test */
    int oom  = 0;
    int ceil = 10240;
    for (int cnt = 0; cnt < ceil; cnt++) {
        int ret = st_slab_obj_alloc(info.slab_pool,
                                    ST_TEST_SLAB_HUGE_OBJ_SIZE,
                                    &addr);
        if (ret != ST_OK) {
            st_ut_eq(ST_OUT_OF_MEMORY, ret, "should be out of memory");
            oom = 1;
            break;
        }
    }

    st_ut_ge(1, oom, "should be out of memory err quit");

    st_list_t *full = &info.slab_pool->groups[ST_SLAB_GROUP_CNT-1].slabs_full;
    while (!st_list_empty(full)) {
        st_list_t *node = st_list_last(full);
        st_pagepool_page_t *master = st_owner(node, st_pagepool_page_t, lnode);
        ret = st_pagepool_page_to_addr(&info.slab_pool->page_pool,
                                       master,
                                       (uint8_t **)&addr);
        st_ut_eq(ST_OK, ret, "failed to convert page to addr");

        st_slab_obj_free(info.slab_pool, addr);
    }

    st_slab_cleanup(&info, 0);
}

#define ST_BENCH_SLAB_MIN_SIZE   1
#define ST_BENCH_SLAB_MAX_SIZE   (st_page_size() * 4)
#define ST_BENCH_SLAB_RESULT_DIR "profile"

typedef struct {
    setup_info_t slab_info;

    ssize_t      th_cnt;     /** the number of thread */
    ssize_t      proc_cnt;   /** the number of processes */
    ssize_t      size_cnt;   /** the number of obj size to test */
    ssize_t      run_times;  /** run times for each size */
    ssize_t      data[0];    /** obj sizes to alloc and free */

} st_slab_prof_t;

static st_slab_prof_t *
st_bench_slab_prof_setup(ssize_t th_cnt,
                         ssize_t proc_cnt,
                         ssize_t size_cnt,
                         ssize_t run_times)
{
    ssize_t data_size    = sizeof(ssize_t) * size_cnt;
    st_slab_prof_t *prof = (st_slab_prof_t *)malloc(sizeof(*prof) + data_size);
    st_assert(prof != NULL);

    prof->th_cnt    = th_cnt;
    prof->proc_cnt  = proc_cnt;
    prof->size_cnt  = size_cnt;
    prof->run_times = run_times;

    if (proc_cnt != 0) {
        st_slab_setup(&prof->slab_info);
    }

    return prof;
}

static inline void
st_bench_slab_prof_cleanup(st_slab_prof_t *prof)
{
    if (prof->proc_cnt != 0) {
        st_slab_cleanup(&prof->slab_info, 1);
    }

    free(prof);
}

static void
st_bench_generate_random_sizes(ssize_t *sizes,
                               int num,
                               ssize_t min_value,
                               ssize_t max_value)
{
    st_assert(min_value < max_value);

    uint32_t rdm;
    int fd = open("/dev/urandom", O_RDONLY);

    st_assert(fd != -1);

    int cnt = 0;
    while (cnt < num) {
        if (sizeof(rdm) != read(fd, &rdm, sizeof(rdm))) {
            continue;
        }

        sizes[cnt++] = (ssize_t)rdm % (max_value + 1 - min_value) + min_value;
    }

    close(fd);
}

static int
set_self_cpu_affinity(int index)
{
    int np = sysconf(_SC_NPROCESSORS_ONLN);

    cpu_set_t mask;

    CPU_ZERO(&mask);
    CPU_SET(index % np, &mask);

    return sched_setaffinity(getpid(), sizeof(mask), &mask);
}

static void
st_bench_slab_do_process_profiling(st_slab_prof_t *prof)
{
    int addr_num = 0;
    void **addrs = (void **)malloc(sizeof(*addrs) * prof->size_cnt);
    st_assert(addrs != NULL);

    int is_alloc    = 1;
    int alloc_times = 0;
    ssize_t rdm_idx = 0;

    for (int cnt = 0; cnt < prof->size_cnt; cnt += alloc_times) {
        alloc_times = 0;
        int ret     = ST_OK;
        ssize_t rdm = prof->data[rdm_idx];

        if (is_alloc) {
            int times = st_min((rdm & 0xff) + 1, prof->size_cnt - cnt);
            for (int num = 0; num < times; num++) {
                ret = st_slab_obj_alloc(prof->slab_info.slab_pool,
                                        prof->data[cnt + num],
                                        &addrs[addr_num]);
                st_assert(ret == ST_OK || ret == ST_OUT_OF_MEMORY);

                if (ret == ST_OK) {
                    alloc_times++;
                    addr_num++;
                } else {
                    break;
                }
            }

        } else {
            int times = st_min((rdm & 0xff) + 1, addr_num);
            for (int num = 0; num < times; num++) {
                st_slab_obj_free(prof->slab_info.slab_pool, addrs[--addr_num]);
            }

        }

        is_alloc = rdm % 2;
        if (addr_num == 0) {
            is_alloc = 1;
        }
        else if (ret == ST_OUT_OF_MEMORY) {
            is_alloc = 0;
        }

        rdm_idx = (rdm_idx + 1) % prof->size_cnt;
    }

    /** free the rest */
    for (int num = 0; num < addr_num; num++) {
        st_slab_obj_free(prof->slab_info.slab_pool, addrs[num]);
    }

    free(addrs);
}

static void
st_bench_slab_profiling_processes(st_slab_prof_t *prof)
{
    if (prof->proc_cnt == 0) {
        return;
    }

    pid_t *children = (pid_t *)malloc(prof->proc_cnt * sizeof(*children));
    st_assert(children != NULL);

    for (int cnt = 0; cnt < prof->proc_cnt; cnt++) {
        children[cnt] = fork();

        if (children[cnt] == -1) {
            derr("failed to fork process\n");
        }
        else if (children[cnt] == 0) {
            /** child process, do test here */
            set_self_cpu_affinity(cnt);

            for (int num = 0; num < prof->run_times; num++) {
                st_bench_slab_do_process_profiling(prof);
            }

            free(children);
            _exit(0);
        }
    }

    for (int cnt = 0; cnt < prof->proc_cnt; cnt++) {
        if (children[cnt] != -1) {
            waitpid(children[cnt], NULL, 0);
        }
    }

    free(children);
}

static void * __attribute__((optimize("O0")))
st_bench_slab_do_thread_profiling(void *arg)
{
    st_slab_prof_t *prof = (st_slab_prof_t *)arg;

    volatile char *ptr = NULL;
    for (int idx = 0; idx < prof->run_times; idx++) {
        for (int cnt = 0; cnt < prof->size_cnt; cnt++) {
            ptr = (char *)malloc(prof->data[cnt]);

            free((char *)ptr);
        }
    }

    return NULL;
}

static void
st_bench_slab_profiling_threads(st_slab_prof_t *prof)
{
    if (prof->th_cnt == 0) {
        return;
    }

    pthread_t *ths = (pthread_t *)malloc(prof->th_cnt * sizeof(*ths));
    st_assert(ths != NULL);

    for (int cnt = 0; cnt < prof->th_cnt; cnt++) {
        int ret = pthread_create(&ths[cnt],
                                 NULL,
                                 st_bench_slab_do_thread_profiling,
                                 prof);
        if (ret != 0) {
            derrno("failed to create thread\n");
        }
    }

    for (int cnt = 0; cnt < prof->th_cnt; cnt++) {
        pthread_join(ths[cnt], NULL);
    }

    free(ths);
}

static void
st_bench_slab_profiling(st_slab_prof_t *prof)
{
    /** generate random test data */
    st_bench_generate_random_sizes(prof->data,
                                   prof->size_cnt,
                                   ST_BENCH_SLAB_MIN_SIZE,
                                   ST_BENCH_SLAB_MAX_SIZE);

    /** fork processes to run profiling st_slab_obj_alloc/free */
    st_bench_slab_profiling_processes(prof);

    /** create threads to run profiling of malloc/free */
    st_bench_slab_profiling_threads(prof);
}

st_ben(st_slab, single_proc, 60, n)
{
    st_slab_prof_t *prof = st_bench_slab_prof_setup(0, 1, 100000, n);

    st_bench_slab_profiling(prof);

    st_bench_slab_prof_cleanup(prof);
}

st_ben(st_slab, multiple_proc, 60, n)
{
    st_slab_prof_t *prof = st_bench_slab_prof_setup(0, 8, 100000, n);

    st_bench_slab_profiling(prof);

    st_bench_slab_prof_cleanup(prof);
}

st_ben(st_slab, single_thread, 60, n)
{
    st_slab_prof_t *prof = st_bench_slab_prof_setup(1, 0, 100000, n);

    st_bench_slab_profiling(prof);

    st_bench_slab_prof_cleanup(prof);
}

st_ben(st_slab, multiple_thread, 60, n)
{
    st_slab_prof_t *prof = st_bench_slab_prof_setup(8, 0, 100000, n);

    st_bench_slab_profiling(prof);

    st_bench_slab_prof_cleanup(prof);
}

st_ut_main;
