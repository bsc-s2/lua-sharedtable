#ifndef ST_SPARSE_SPARSE_H_
#define ST_SPARSE_SPARSE_H_

#include "inc/inc.h"

typedef struct st_sparse_array_s st_sparse_array_t;

struct st_sparse_array_s {

    uint16_t  cnt;        /* number of elts in this sparse array. */
    uint16_t  capacity;
    uint16_t  elt_size;

    uint64_t *bitmaps;    /* bitmaps[] about which index has elt  */
    uint16_t *offsets;    /* index offset in `elts` for bitmap[i] */
    uint8_t  *elts;
};

#define st_sparse_array_arg(cnt_, capacity_, elt_size_)                       \
        {                                                                     \
            .cnt = (cnt_),                                                    \
            .capacity = (capacity_),                                          \
            .elt_size = (elt_size_),                                          \
        }

#define st_sparse_array_bitmap_width(sa) (sizeof((sa)->bitmaps[0]) * 8)

int st_sparse_array_new(st_sparse_array_t arg,
                        uint16_t *index,
                        uint8_t **elts,
                        st_sparse_array_t **rst);

/* There is not a destory function for sparse array, just free.
 * Because it is a continous allocated memory.
 */

uint8_t *st_sparse_array_get(st_sparse_array_t *sa, ssize_t i);


#endif /* ST_SPARSE_SPARSE_H_ */
