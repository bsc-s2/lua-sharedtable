<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
#   Table of Content

- [Name](#name)
- [Status](#status)
- [Synopsis](#synopsis)
- [Description](#description)
- [DataTypes](#structure)
- [Author](#author)
- [Copyright and License](#copyright-and-license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Name

slab

# Status

This library is considered production ready.

# Synopsis

Usage of slab module.
The following code just simply demonstrates the usage, you may need your own error handling.

```
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


#define PAGE_SIZE                        (1 << 12)
#define ST_TEST_SLAB_HUGE_OBJ_SIZE       (PAGE_SIZE + 1)

#define ST_TEST_SLAB_REGION_CNT          10
#define ST_TEST_SLAB_SHARED_SPACE_LENGTH (1024 * 1024 * 200)

static void
st_slab_setup(setup_info_t *info)
{
    int ret = st_region_shm_create(ST_TEST_SLAB_SHARED_SPACE_LENGTH,
                                   &info->base,
                                   &info->shm_fd);
    st_assert(ret == ST_OK);

    info->slab_pool = (st_slab_pool_t *)info->base;
    info->data = (void *)(st_align((uintptr_t)info->base + sizeof(*info->slab_pool), PAGE_SIZE));

    ssize_t cfg_len = (uintptr_t)info->data - (uintptr_t)info->base;
    ssize_t data_len = ST_TEST_SLAB_SHARED_SPACE_LENGTH - cfg_len;

    ret = st_region_init(&info->slab_pool->page_pool.region_cb,
                         info->data,
                         data_len / PAGE_SIZE / ST_TEST_SLAB_REGION_CNT,
                         ST_TEST_SLAB_REGION_CNT,
                         1);
    st_assert(ret == ST_OK);

    ret = st_pagepool_init(&info->slab_pool->page_pool, PAGE_SIZE);
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

int
main(int argc, char *argv[])
{
    /** setup slab */
    setup_info_t info;
    st_slab_setup(&info);

    /** specific memory size */
    ssize_t sizes[] = {4, 8, 16, 2048, 3000};

    for (int cnt = 0; cnt < st_nelts(sizes); cnt++) {
        void *addr = NULL;

        /** alloc memory from slab */
        int ret = st_slab_obj_alloc(info->slab_pool, sizes[cnt], &addr);
        if (ret != ST_OK) {
            derr("failed to alloc mem from slab %d\n", ret);
            /** error handling code */
            continue;
        }

        /** use addr as you wish */

        /** free memory */
        ret = st_slab_obj_free(info->slab_pool, addr);
        if (ret != ST_OK) {
            derr("failed to free memory to slab %d\n", ret);
            /** error handling code */
        }
    }

    /** cleanup and destroy slab */
    st_slab_cleanup(&info, 1);

    return 0;
}
```

# Description
slab is a kind of special memory cache and it bases on pagepool module which means it allocates memory
from pagepool and frees memory to pagepool.

It caches multiple fixed-size objects. Objects of the same size are originized into the same group.
Different groups manages objects of different sizes.

Caller can initialize slab by `st_slab_pool_init()` and destroy it by `st_slab_pool_destroy()` interface.

After initialization, caller can require objects of arbitrary size from slab by `st_slab_obj_alloc()`, and
free objects by `st_slab_obj_free()`. These two functions just like `malloc()` and `free()`.

# DataTypes
```
typedef struct {
    union {
        ssize_t times;
        ssize_t cnt;
    } free;
    union {
        ssize_t times;
        ssize_t cnt;
    } alloc;
} st_slab_group_stat_t;

```
- `free.times`: free objects/pages times.
- `free.cnt`: the number of free state objects cached in slab group.
- `alloc.times`: allocate objects/pages times.
- `alloc.cnt`: the number of allocated state objects cached in slab group.

```
struct st_slab_group_s {
    struct {
        /** total obj free and allocation times */
        st_slab_group_stat_t obj;

        /** total pages free and allocation times */
        st_slab_group_stat_t pages;

        /** current objs number in free/alloc state */
        st_slab_group_stat_t current;
    } stat;
    struct timeval      start_time;

    ssize_t             obj_size;
    st_list_t           slabs_full;
    st_list_t           slabs_partial;
    st_pagepool_t       *page_pool;

    int                 inited;
    pthread_mutex_t     lock;
};
```
- `stat`: currently group state
- `stat.obj`: total times of objects allocation and free.
- `stat.pages`: total times of compound pages allocation and free.
- `stat.current`: the number of objs in free and allocated state.
- `start_time`: the initialization time of slab group.
- `obj_size`: the size of objects managed by this group.
- `slabs_full`: the list of compound pages in full state.
- `slabs_partial`: the list of compound pages which still has free memory space.
- `page_pool`: pointer to page pool handler.
- `inited`: is this group already initialized or not.
- `lock`: lock to protect slabs_full and slabs_partial.


```
struct st_slab_pool_s {
    st_pagepool_t   page_pool;

    /**
     * the last one is for huge objects whose size is greater than
     * 1 << ST_SLAB_OBJ_SIZE_MAX_SHIFT.
     *
     * huge obj size recorded in group is ST_SLAB_HUGE_OBJ_SIZE
     */
    st_slab_group_t groups[ST_SLAB_GROUP_CNT];
};
```
- `page_pool`: memory space for page pool handler.
- `groups`: slab groups array.

# Author

李树龙 <shulong.li@baishancloud.com>

# Copyright and License

The MIT License (MIT)

Copyright (c) 2017 李树龙 <shulong.li@baishancloud.com>
