#include "sparse.h"
#include "unittest/unittest.h"


st_test(sparse, new)
{

    st_sparse_array_t arg = st_sparse_array_arg(5, 120, 2);
    uint16_t index[5] = {1, 2, 64, 65, 66};
    uint8_t *elts[5] = {
        (uint8_t *)"ab",
        (uint8_t *)"cd",
        (uint8_t *)"ef",
        (uint8_t *)"gh",
        (uint8_t *)"ij",
    };
    st_sparse_array_t *sa;
    int ret;

    st_ut_bug(st_sparse_array_new(arg, NULL, elts, &sa));
    st_ut_bug(st_sparse_array_new(arg, index, NULL, &sa));
    st_ut_bug(st_sparse_array_new(arg, index, elts, NULL));

    ret = st_sparse_array_new(arg, index, elts, &sa);
    st_ut_eq(ST_OK, ret);
    st_ut_ne(NULL, sa);

    st_ut_eq(5, sa->cnt);
    st_ut_eq(120, sa->capacity);
    st_ut_eq(2, sa->elt_size);

    st_ut_eq(st_align(sizeof(*sa),
                      sizeof(uint64_t)),
             (uint8_t *)sa->bitmaps - (uint8_t *)sa);

    st_ut_eq(st_align(sizeof(sa->bitmaps[0]) * 2,
                      sizeof(uint64_t)),
             (uint8_t *)sa->offsets - (uint8_t *)sa->bitmaps);

    st_ut_eq(st_align(sizeof(sa->offsets[0]) * 2,
                      sizeof(uint64_t)),
             (uint8_t *)sa->elts - (uint8_t *)sa->offsets);

    /* check bitmaps */

    st_ut_eq(0x6ULL, sa->bitmaps[0]);
    st_ut_eq(0x7ULL, sa->bitmaps[1]);

    /* check offsets */

    st_ut_eq((uint16_t)0, sa->offsets[0]);
    st_ut_eq((uint16_t)2, sa->offsets[1]);

    /* check elts content */

    st_ut_eq(0, memcmp("abcdefghij", sa->elts, 10), "content should be abcdefghij");

    st_free(sa);
}

st_test(sparse, get)
{

    st_sparse_array_t arg = st_sparse_array_arg(5, 120, 2);

    uint16_t index[5] = {1, 2, 64, 65, 66};
    uint8_t *elts[5] = {
        (uint8_t *)"ab",
        (uint8_t *)"cd",
        (uint8_t *)"ef",
        (uint8_t *)"gh",
        (uint8_t *)"ij",
    };
    st_sparse_array_t *sa;
    int ret;

    ret = st_sparse_array_new(arg, index, elts, &sa);
    st_ut_eq(ST_OK, ret);

    struct case_s {
        ssize_t  i;
        char    *expected;
    } cases[] = {
        {0,  NULL},
        {1,  "ab"},
        {2,  "cd"},
        {3,  NULL},
        {63, NULL},
        {64, "ef"},
        {65, "gh"},
        {66, "ij"},
        {67, NULL},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_autotype c   = cases[i];
        st_autotype rst = st_sparse_array_get(sa, c.i);

        utddx(c.i);
        utddx(c.expected);
        utddx(rst);

        if (c.expected == NULL) {
            st_ut_eq(NULL, rst);
        }
        else {
            st_ut_eq(0, memcmp(c.expected, rst, strlen(c.expected)));
        }
    }

    st_free(sa);
}

st_ut_main;
