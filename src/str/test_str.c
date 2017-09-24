#include "str.h"
#include "unittest/unittest.h"

#define literal "abc"

st_test(str, const) {

    {
        st_str_t s = st_str_const(literal);

        st_ut_eq(NULL,              s.left,        "left is NULL");
        st_ut_eq(sizeof(literal)-1, s.len,         "len");
        st_ut_eq(sizeof(literal),   s.capacity,    "capacity");
        st_ut_eq(0,                 s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL,              s.bytes,       "bytes is not NULL");
    }

    {
        st_str_t s = st_str_var(10);

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(10,   s.len,         "len");
        st_ut_eq(10,   s.capacity,    "capacity");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL, s.bytes,       "bytes is not NULL");
    }

    {
        char *b = literal;
        int64_t l = 2;
        st_str_t s = st_str_wrap(b, l);

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(l,    s.len,         "len");
        st_ut_eq(l,    s.capacity,    "capacity");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(b,    s.bytes,       "bytes is not NULL");
    }

    {
        char *b = literal;
        int64_t l = sizeof(literal) - 1;
        st_str_t s = st_str_wrap_0(b, l);

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(l,    s.len,         "len");
        st_ut_eq(l+1,  s.capacity,    "capacity");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(b,    s.bytes,       "bytes is b");

        st_ut_eq(1, st_str_trailing_0(&s), "it has trailing 0");
    }

    {
        char b[10];
        st_str_t s = st_str_wrap_chars(b);

        st_ut_eq(NULL,        s.left,        "left is NULL");
        st_ut_eq(10,          s.len,         "len");
        st_ut_eq(10,          s.capacity,    "capacity");
        st_ut_eq(0,           s.bytes_owned, "bytes_owned is 0");
        st_ut_eq((uint8_t*)b, s.bytes,       "bytes is b");
    }

    {
        st_str_t s = st_str_null;

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(0,    s.len,         "len");
        st_ut_eq(0,    s.capacity,    "capacity");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL, s.bytes,       "bytes is NULL");
    }

    {
        st_str_t s = st_str_empty;

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(0,    s.len,         "len");
        st_ut_eq(1,    s.capacity,    "capacity");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL, s.bytes,       "bytes is not NULL");
    }

}

st_test(str, init_invalid) {

    struct case_s {
        uint64_t inp;
        int expected;
    } cases[] = {
        {-1, ST_ARG_INVALID},
        {-2, ST_ARG_INVALID},
        {-1024, ST_ARG_INVALID},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst;

        st_str_t s = {0};

        rst = st_str_init(&s, c.inp);
        ddx(rst);

        st_ut_eq(c.expected, rst, "");

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(NULL, s.right,       "right is NULL");
        st_ut_eq(0,    s.len,         "len is 0");
        st_ut_eq(0,    s.capacity,    "capacity is 0");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL, s.bytes,       "bytes is NULL");
    }
}

st_test(str, init) {

    int ret;

    {
        st_str_t s = st_str_null;
        ret = st_str_init(&s, 0);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(0,    s.len,         "len is 0");
        st_ut_eq(1,    s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0, use pre-allocated empty c-string");
        st_ut_ne(NULL, s.bytes,       "bytes is not NULL");

        ret = st_str_destroy(&s);
        st_ut_eq(ST_OK, ret, "destroy");

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(0,    s.len,         "len is 0");
        st_ut_eq(0,    s.capacity,    "capacity is 0");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL, s.bytes,       "bytes is NULL");
    }

    {
        st_str_t s = st_str_null;
        ret = st_str_init(&s, 1);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(NULL, s.left,        "left is NULL");
        st_ut_eq(1,    s.len,         "len is 0");
        st_ut_eq(1,    s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(1,    s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL, s.bytes,       "bytes is not NULL");

        ret = st_str_destroy(&s);
        st_ut_eq(ST_OK, ret, "destroy");
    }

    {
        st_str_t s = st_str_null;
        ret = st_str_init(&s, 102400);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(NULL,   s.left,        "left is NULL");
        st_ut_eq(102400, s.len,         "len is 0");
        st_ut_eq(102400, s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(1,      s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL,   s.bytes,       "bytes is not NULL");

        ret = st_str_destroy(&s);
        st_ut_eq(ST_OK, ret, "destroy");
    }
}

/* test init_0 invalid and valid */
/* test equal */
/* test n_arr */
/* test trailing_0 */
/* test destroy */
/* test ref */
/* test copy */
/* test copy_cstr */
/* test cmp */
/* test seize */
st_ut_main;
