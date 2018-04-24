#include "sparse.h"

static void
st_sparse_array_append(st_sparse_array_t *sa, ssize_t i, uint8_t *elt)
{

    st_assert(sa != NULL);
    st_assert(i < sa->capacity);
    st_assert(elt != NULL);

    ddx(i);

    ssize_t ibitmap = i / st_sparse_array_bitmap_width(sa);
    ssize_t ibit    = i % st_sparse_array_bitmap_width(sa);
    ddx(ibitmap);
    ddx(ibit);

    uint64_t *bitmap_word = &sa->bitmaps[ibitmap];

    if (*bitmap_word == 0) {
        sa->offsets[ibitmap] = sa->cnt;
        ddx(sa->offsets[ibitmap]);
    }

    *bitmap_word |= 1ULL << ibit;
    ddx(bitmap_word);
    ddx(*bitmap_word);
    dd("elt=%s", elt);
    st_memcpy(&sa->elts[sa->elt_size * sa->cnt], elt, sa->elt_size);

    sa->cnt++;
    ddx(sa->cnt);
}

/* `arg` is used as sparse array creation argument,
 * and has only cnt, capacity, and elt_size set
 */
int
st_sparse_array_new(st_sparse_array_t arg,
                    uint16_t *index,
                    uint8_t **elts,
                    st_sparse_array_t **rst)
{
    st_assert(index != NULL);
    st_assert(elts != NULL);
    st_assert(rst != NULL);

    ssize_t i;
    st_sparse_array_t x; /* a placeholder for accessing fields easily */
    st_sparse_array_t *r; /* intermediate result */

    ssize_t bm_width       = st_sparse_array_bitmap_width(&arg);
    ssize_t bm_cnt         = st_align(arg.capacity, bm_width) / bm_width;
    ssize_t bm_size        = sizeof(x.bitmaps[0]) * bm_cnt;
    ssize_t offset_size    = sizeof(x.offsets[0]) * bm_cnt;
    ssize_t elt_total_size = arg.elt_size * arg.cnt;

    /* bitmaps, offsets and elts should be 8-byte aligned for better perf */

    ssize_t bm_offset = st_align(sizeof(st_sparse_array_t),
                                 sizeof(uint64_t));

    ssize_t os_offset = st_align(bm_offset + bm_size,
                                 sizeof(uint64_t));

    ssize_t elts_offset = st_align(os_offset + offset_size,
                                   sizeof(uint64_t));

    ssize_t total_size = elts_offset + elt_total_size;

    r = st_malloc(total_size);
    if (r == NULL) {
        return ST_OUT_OF_MEMORY;
    }

    bzero(r, total_size);

    *r = arg;

    uint8_t *base = (uint8_t *)r;
    r->bitmaps = (uint64_t *)&base[bm_offset];
    r->offsets = (uint16_t *)&base[os_offset];
    r->elts = &base[elts_offset];

    r->cnt = 0;
    for (i = 0; i < arg.cnt; i++) {
        ssize_t idx = index[i];
        uint8_t *elt = elts[i];
        st_sparse_array_append(r, idx, elt);
    }

    *rst = r;

    return ST_OK;
}

uint8_t *
st_sparse_array_get(st_sparse_array_t *sa, ssize_t i)
{

    ssize_t ibitmap = i / st_sparse_array_bitmap_width(sa);
    ssize_t ibit    = i % st_sparse_array_bitmap_width(sa);

    uint64_t bitmap_word = sa->bitmaps[ibitmap];

    if ((bitmap_word >> ibit) & 1) {
        ssize_t base = sa->offsets[ibitmap];
        ssize_t cnt_1 = st_bit_cnt1_before(bitmap_word, ibit);

        return &sa->elts[sa->elt_size * (base + cnt_1)];
    }
    else {
        return NULL;
    }
}
