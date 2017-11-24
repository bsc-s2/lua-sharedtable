#ifndef __ST_SLAB_H_INCLUDE__
#define __ST_SLAB_H_INCLUDE__


#include <limits.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <inttypes.h>

#include "inc/bit.h"
#include "inc/err.h"
#include "inc/util.h"
#include "list/list.h"
#include "bitmap/bitmap.h"
#include "pagepool/pagepool.h"
#include "robustlock/robustlock.h"


#define ADDR_PAGE_BASE(addr, page_size) \
    ((uint8_t *)((uintptr_t)addr & ~((page_size) - 1)))

#define ST_SLAB_OBJ_SIZE_MIN_SHIFT    3
#define ST_SLAB_OBJ_SIZE_MAX_SHIFT    11
#define ST_SLAB_OBJ_CNT_EACH_ALLOC    512

/**
 * in huge object group, each object may have different size,
 * so use this size to mark huge object group
 */
#define ST_SLAB_HUGE_OBJ_SIZE    (1 << (ST_SLAB_OBJ_SIZE_MAX_SHIFT + 1))

/** group size: (2^0) ~ (2^ST_SLAB_OBJ_SIZE_MAX_SHIFT) + huge obj group */
#define ST_SLAB_GROUP_CNT             (ST_SLAB_OBJ_SIZE_MAX_SHIFT + 2)


typedef struct st_slab_group_s st_slab_group_t;
typedef struct st_slab_pool_s  st_slab_pool_t;

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

/**
 * object:
 *     called obj for short.
 *     object means a fixed size memory block.
 *
 * slab:
 *     slab contains a fixed number (ST_SLAB_OBJ_CNT_EACH_ALLOC) of objects.
 *
 * slab_group:
 *     slab group contains multiple slabs, and the number of slabs
 *     is determined by runtime allocation and free.
 *
 *     each slab is either in slabs_full list when all objects in slab are
 *     allocated to user or in slabs_partial list when there is(are) still
 *     free objects in it.
 */
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


/**
 * page_pool inside slab_pool must be initialized
 * before calling st_slab_init
 *
 * state is recorded in slab_pool object,
 * so it must be memset to all zero when first called.
 *
 * this function is idempotent.
 */
int st_slab_pool_init(st_slab_pool_t *slab_pool);

/**
 * if slab_pool is never initialized, the behavior is undefined.
 *
 * this function is idempotent.
 */
int st_slab_pool_destroy(st_slab_pool_t *slab_pool);

/**
 * alloc an object of size bytes
 */
int st_slab_obj_alloc(st_slab_pool_t *slab_pool, ssize_t size, void **ret_addr);

/**
 * free the memory space of pointed to by addr which must be allocated by
 * st_slab_obj_alloc before. otherwise, or if addr is freed before,
 * undefined behavior occurs.
 *
 * so caller must guarantee that addr is a valid object address in slab
 */
int st_slab_obj_free(st_slab_pool_t *slab_pool, void *addr);

#endif
