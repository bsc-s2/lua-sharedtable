#include "str.h"
#include "unittest/unittest.h"
#include <limits.h>
#include <float.h>

#define FOO "abc"
#define ST_TEST_EXPECT_PANIC 1

st_test(str, const) {

    {
        st_str_t s = st_str_const(FOO);

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(sizeof(FOO)-1,   s.len,         "len");
        st_ut_eq(sizeof(FOO),     s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");
    }

    {
        st_str_t s = st_str_local(10);

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(10,              s.len,         "len");
        st_ut_eq(10,              s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");
    }

    {
        char     *b = FOO;
        int64_t   l = 2;
        st_str_t  s = st_str_wrap(b, l);

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(l,               s.len,         "len");
        st_ut_eq(l,               s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(b,               s.bytes,       "bytes is not NULL");
    }

    {
        char     *b = FOO;
        int64_t   l = sizeof(FOO) - 1;
        st_str_t  s = st_str_wrap_0(b, l);

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(l,               s.len,         "len");
        st_ut_eq(l+1,             s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(b,               s.bytes,       "bytes is b");

        st_ut_eq(1, st_str_trailing_0(&s), "it has trailing 0");
    }

    {
        char b[10];
        st_str_t s = st_str_wrap_chars(b);

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(10,              s.len,         "len");
        st_ut_eq(10,              s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_eq((uint8_t*)b,     s.bytes,       "bytes is b");
    }

    {
        st_str_t s = st_str_zero;

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(0,               s.len,         "len");
        st_ut_eq(0,               s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL,            s.bytes,       "bytes is NULL");
    }

    {
        st_str_t s = st_str_empty;

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(0,               s.len,         "len");
        st_ut_eq(1,               s.capacity,    "capacity");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");
    }

}

st_test(str, init_invalid) {

    {
        int ret = st_str_init(NULL, 1);
        st_ut_eq(ST_ARG_INVALID, ret, "s is NULL");
    }

    struct case_s {
        int64_t inp;
        int expected;
    } cases[] = {
        {-1, ST_ARG_INVALID},
        {-2, ST_ARG_INVALID},
        {-1024, ST_ARG_INVALID},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst;

        st_str_t s = {{0}};

        rst = st_str_init(&s, c.inp);
        ddx(rst);

        st_ut_eq(c.expected, rst, "");

        st_ut_eq(0,    s.type,        "type is 0");
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
        st_str_t s = st_str_zero;
        ret = st_str_init(&s, 0);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(0,               s.len,         "len is 0");
        st_ut_eq(1,               s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(0,               s.bytes_owned, "bytes_owned is 0, use pre-allocated empty c-string");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);

        st_ut_eq(-1,   s.type,        "type is -1");
        st_ut_eq(0,    s.len,         "len is 0");
        st_ut_eq(0,    s.capacity,    "capacity is 0");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL, s.bytes,       "bytes is NULL");
    }

    {
        st_str_t s = st_str_zero;
        ret = st_str_init(&s, 1);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(1,               s.len,         "len is 0");
        st_ut_eq(1,               s.capacity,    "capacity is 1");
        st_ut_eq(1,               s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);
    }

    {
        st_str_t s = st_str_zero;
        ret = st_str_init(&s, 102400);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(102400,          s.len,         "len is 0");
        st_ut_eq(102400,          s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(1,               s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);
    }
}

st_test(str, init_0) {

    int ret;

    {
        st_str_t s = st_str_zero;
        ret = st_str_init_0(&s, 0);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(0,               s.len,         "len is 0");
        st_ut_eq(1,               s.capacity,    "capacity is 1, because it use a pre-allocated empty c-string");
        st_ut_eq(1,               s.bytes_owned, "bytes_owned is 0, use pre-allocated empty c-string");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);
    }

    {
        st_str_t s = st_str_zero;
        ret = st_str_init_0(&s, 1);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(1,               s.len,         "len");
        st_ut_eq(2,               s.capacity,    "capacity is 2");
        st_ut_eq(1,               s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);
    }

    {
        st_str_t s = st_str_zero;
        ret = st_str_init_0(&s, 102400);
        st_ut_eq(ST_OK, ret, "init ok");

        st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
        st_ut_eq(102400,          s.len,         "len");
        st_ut_eq(102401,          s.capacity,    "capacity");
        st_ut_eq(1,               s.bytes_owned, "bytes_owned is 1");
        st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

        st_str_destroy(&s);
    }
}

st_test(str, init_0_invalid) {

    {
        int ret = st_str_init_0(NULL, 1);
        st_ut_eq(ST_ARG_INVALID, ret, "s is NULL");
    }


    struct case_s {
        int64_t inp;
        int expected;
    } cases[] = {
        {-1, ST_ARG_INVALID},
        {-2, ST_ARG_INVALID},
        {-1024, ST_ARG_INVALID},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst;

        st_str_t s = {{0}};

        rst = st_str_init_0(&s, c.inp);

        st_ut_eq(c.expected, rst, "");

        st_ut_eq(0,    s.type,        "type is 0");
        st_ut_eq(NULL, s.right,       "right is NULL");
        st_ut_eq(0,    s.len,         "len is 0");
        st_ut_eq(0,    s.capacity,    "capacity is 0");
        st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
        st_ut_eq(NULL, s.bytes,       "bytes is NULL");
    }
}

st_test(str, equal) {

    {
        st_str_t *emp = &(st_str_t)st_str_empty;
        st_str_t *nu = &(st_str_t)st_str_zero;

        st_ut_eq(0, st_str_equal(NULL, emp), "");
        st_ut_eq(0, st_str_equal(emp, NULL), "");
        st_ut_eq(0, st_str_equal(NULL, nu), "");
        st_ut_eq(0, st_str_equal(nu, NULL), "");
        st_ut_eq(0, st_str_equal(emp, nu), "");
        st_ut_eq(0, st_str_equal(nu, emp), "");

    }

    struct case_s {
        char *a;
        char *b;
        int expected;
    } cases[] = {
        {"",   "",   1},
        {"",   "a",  0},
        {"a",  "",   0},
        {"a",  "a",  1},
        {"aa", "a",  0},
        {"a",  "aa", 0},
        {"aa", "aa", 1},
        {"ab", "aa", 0},
        {"aa", "ab", 0},
        {"ab", "ab", 1},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst;

        st_str_t a = st_str_wrap(c.a, strlen(c.a));
        st_str_t b = st_str_wrap(c.b, strlen(c.b));
        rst = st_str_equal(&a, &b);

        ddx(rst);

        st_ut_eq(c.expected, rst, "");
    }
}

st_test(str, cmp) {

    {
        st_str_t *emp = &(st_str_t)st_str_empty;
        st_str_t *nu  = &(st_str_t)st_str_zero;

        st_ut_eq(ST_ARG_INVALID, st_str_cmp(NULL, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_cmp(NULL, emp),  "");
        st_ut_eq(ST_ARG_INVALID, st_str_cmp(emp,  NULL), "");

        st_ut_eq(1,  st_str_cmp(emp, nu),  "");
        st_ut_eq(0,  st_str_cmp(emp, emp), "");
        st_ut_eq(-1, st_str_cmp(nu,  emp), "");
    }

    {
        st_str_t *emp1 = &(st_str_t)st_str_empty;
        st_str_t *emp2 = &(st_str_t)st_str_empty;

        st_ut_eq(0, st_str_cmp(emp1, emp2), "");

        emp1->type = 1;
        emp2->type = 0;
        st_ut_eq(1, st_str_cmp(emp1, emp2), "");

        emp1->type = 0;
        emp2->type = 1;
        st_ut_eq(-1, st_str_cmp(emp1, emp2), "");
    }

    struct case_s {
        char *a;
        char *b;
        int expected;
    } cases[] = {
        {"",   "",   0},
        {"",   "a",  -1},
        {"a",  "",   1},
        {"a",  "a",  0},
        {"aa", "a",  1},
        {"a",  "aa", -1},
        {"aa", "aa", 0},
        {"ab", "aa", 1},
        {"aa", "ab", -1},
        {"ab", "ab", 0},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst;

        st_str_t a = st_str_wrap(c.a, strlen(c.a));
        st_str_t b = st_str_wrap(c.b, strlen(c.b));
        rst = st_str_cmp(&a, &b);

        ddx(rst);

        st_ut_eq(c.expected, rst, "");
    }

    {
        /** invalid ST_TYPES_TABLE */
        st_str_t a = st_str_wrap_common(NULL, ST_TYPES_TABLE, 0);
        st_str_t b = st_str_wrap_common(NULL, ST_TYPES_INTEGER, 0);

        int rst = st_str_cmp(&a, &b);
        st_ut_eq(ST_ARG_INVALID, rst, "table type should be invalid");

        rst = st_str_cmp(&b, &a);
        st_ut_eq(ST_ARG_INVALID, rst, "table type should be invalid");

    }

    {
        /** cmp by different types */
        for (int t = ST_TYPES_INTEGER; t < ST_TYPES_NIL; t++) {
            st_str_t a = st_str_wrap_common(NULL, t, 0);
            st_str_t b = st_str_wrap_common(NULL, t + 1, 0);

            int rst = st_str_cmp(&a, &b);
            st_ut_eq(-1, rst, "failed to compare types");
        }
    }

    {
        /** int */
        int values[] = { INT_MIN, -256, -255, -254, -1, 0, 255, INT_MAX-1 };
        for (int idx = 0; idx < st_nelts(values); idx++) {
            int ia = values[idx];
            int ib = ia + 1;
            st_str_t a = st_str_wrap_common(&ia, ST_TYPES_INTEGER, sizeof(ia));
            st_str_t b = st_str_wrap_common(&ib, ST_TYPES_INTEGER, sizeof(ib));

            int rst = st_str_cmp(&a, &b);
            st_ut_eq(-1, rst, "failed to compare int type lt case");

            rst = st_str_cmp(&a, &a);
            st_ut_eq(0, rst, "failed to compare int type eq case");

            rst = st_str_cmp(&b, &a);
            st_ut_eq(1, rst, "failed to compare int type gt case");
        }
    }

    {
        /** uint64_t */
        uint64_t values[] = { 0, 255, ULONG_MAX-1 };
        for (int idx = 0; idx < st_nelts(values); idx++) {
            uint64_t u64_a = values[idx];
            uint64_t u64_b = u64_a + 1;

            st_str_t a = st_str_wrap_common(&u64_a, ST_TYPES_U64, sizeof(u64_a));
            st_str_t b = st_str_wrap_common(&u64_b, ST_TYPES_U64, sizeof(u64_b));

            int rst = st_str_cmp(&a, &b);
            st_ut_eq(-1, rst, "failed to compare uint64_t type lt case");

            rst = st_str_cmp(&a, &a);
            st_ut_eq(0, rst, "failed to compare uint64_t type eq case");

            rst = st_str_cmp(&b, &a);
            st_ut_eq(1, rst, "failed to compare uint64_t type gt case");
        }
    }

    {
        /** double */
        double values[] = { -256.0, -255.0, 0.0, 255.0 };
        for (int idx = 0; idx < st_nelts(values); idx++) {
            double da = values[idx];
            double db = da + 1;

            st_str_t a = st_str_wrap_common(&da, ST_TYPES_NUMBER, sizeof(da));
            st_str_t b = st_str_wrap_common(&db, ST_TYPES_NUMBER, sizeof(db));

            int rst = st_str_cmp(&a, &b);
            st_ut_eq(-1, rst, "failed to compare double type lt case");

            rst = st_str_cmp(&a, &a);
            st_ut_eq(0, rst, "failed to compare double type eq case");

            rst = st_str_cmp(&b, &a);
            st_ut_eq(1, rst, "failed to compare double type gt case");
        }
    }

    {
        /** st_bool */
        st_bool ba = 0;
        st_bool bb = 1;

        st_str_t a = st_str_wrap_common(&ba, ST_TYPES_BOOLEAN, sizeof(ba));
        st_str_t b = st_str_wrap_common(&bb, ST_TYPES_BOOLEAN, sizeof(bb));

        int rst = st_str_cmp(&a, &b);
        st_ut_eq(-1, rst, "failed to compare st_bool type lt case");

        rst = st_str_cmp(&a, &a);
        st_ut_eq(0, rst, "failed to compare st_bool type eq case");

        rst = st_str_cmp(&b, &a);
        st_ut_eq(1, rst, "failed to compare st_bool type gt case");

    }

    {
        /** null */
        st_str_t a = st_str_wrap_common(NULL, ST_TYPES_NIL, 0);
        st_str_t b = st_str_wrap_common(NULL, ST_TYPES_NIL, 0);

        int rst = st_str_cmp(&a, &b);
        st_ut_eq(0, rst, "failed to comapre nil type case");
    }
}

st_test(str, trailing_0) {

    st_str_t s = st_str_const(FOO);
    st_ut_eq(1, st_str_trailing_0(&s), "");

    s.len += 1;
    st_ut_eq(0, st_str_trailing_0(&s), "");

    /* len = capacity-2, invalid str */
    s.len -= 2;
    st_ut_eq(0, st_str_trailing_0(&s), "");
}

st_test(str, destroy) {

    {
        st_ut_bug(st_str_destroy(NULL));
    }

    int ret;

    st_str_t s = st_str_zero;
    ret = st_str_init(&s, 10);
    st_ut_eq(ST_OK, ret, "init ok");

    st_ut_eq(ST_TYPES_STRING, s.type,        "type is not string");
    st_ut_eq(10,              s.len,         "len is 10");
    st_ut_eq(10,              s.capacity,    "capacity is 10");
    st_ut_eq(1,               s.bytes_owned, "bytes_owned is 1");
    st_ut_ne(NULL,            s.bytes,       "bytes is not NULL");

    st_str_destroy(&s);

    st_ut_eq(-1,   s.type,        "type is -1");
    st_ut_eq(0,    s.len,         "len is 0");
    st_ut_eq(0,    s.capacity,    "capacity is 0");
    st_ut_eq(0,    s.bytes_owned, "bytes_owned is 0");
    st_ut_eq(NULL, s.bytes,       "bytes is NULL");
}

st_test(str, ref) {

    {
        st_str_t s = st_str_zero;
        st_str_t t = st_str_zero;

        st_ut_bug(st_str_ref(NULL, NULL), "");
        st_ut_bug(st_str_ref(NULL, &s), "");
        st_ut_bug(st_str_ref(&s, NULL), "");
        st_ut_bug(st_str_ref(&s, &s), "");

        s.bytes_owned = 1;
        st_ut_bug(st_str_ref(&s, &t), "");
    }

    int ret;

    st_str_t s = st_str_zero;
    ret = st_str_init(&s, 10);
    st_ut_eq(ST_OK, ret, "init ok");

    st_str_t ref = st_str_zero;

    st_str_ref(&ref, &s);

    st_ut_eq(1, s.bytes_owned, "target bytes_owned does not change");

    st_ut_eq(ST_TYPES_STRING, ref.type,        "type is not string");
    st_ut_eq(10,              ref.len,         "len is 0");
    st_ut_eq(10,              ref.capacity,    "capacity is 0");
    st_ut_eq(0,               ref.bytes_owned, "bytes_owned is 0, ref does not need to free memory");
    st_ut_eq(s.bytes,         ref.bytes,       "bytes is shared");

    st_str_destroy(&s);

    st_str_destroy(&ref);
}

st_test(str, copy) {

    {
        st_str_t s = st_str_zero;
        st_str_t t = st_str_zero;

        st_ut_eq(ST_ARG_INVALID, st_str_copy(NULL, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_copy(NULL, &s), "");
        st_ut_eq(ST_ARG_INVALID, st_str_copy(&s, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_copy(&s, &s), "");

        s.bytes_owned = 1;
        st_ut_bug(st_str_ref(&s, &t), "");
    }

    int ret;

    st_str_t s = st_str_zero;
    ret = st_str_init_0(&s, 10);
    st_ut_eq(ST_OK, ret, "init ok");

    st_str_t cpy = st_str_zero;

    ret = st_str_copy(&cpy, &s);
    st_ut_eq(ST_OK, ret, "");

    st_ut_eq(1, s.bytes_owned, "target bytes_owned does not change");

    st_ut_eq(ST_TYPES_STRING, cpy.type,        "type is not string");
    st_ut_eq(s.len,           cpy.len,         "len is same as s");
    st_ut_eq(s.capacity,      cpy.capacity,    "capacity is same as s");
    st_ut_eq(1,               cpy.bytes_owned, "bytes_owned is 1, memory reallocated");
    st_ut_ne(s.bytes,         cpy.bytes,       "bytes is not shared");

    ret = st_memcmp(s.bytes, cpy.bytes, s.capacity);
    st_ut_eq(0, ret, "cpy.bytes is same as s.bytes");

    st_str_destroy(&s);
    st_str_destroy(&cpy);
}

st_test(str, copy_cstr) {

    {
        st_str_t s = st_str_zero;

        st_ut_eq(ST_ARG_INVALID, st_str_copy_cstr(NULL, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_copy_cstr(NULL, FOO), "");
        st_ut_eq(ST_ARG_INVALID, st_str_copy_cstr(&s, NULL), "");

        s.bytes_owned = 1;
        st_ut_eq(ST_INITTWICE, st_str_copy_cstr(&s, FOO), "");
    }

    int ret;

    st_str_t s = st_str_zero;

    ret = st_str_copy_cstr(&s, FOO);
    st_ut_eq(ST_OK, ret, "");

    st_ut_eq(0,             s.type,        "type is 0");
    st_ut_eq(sizeof(FOO)-1, s.len,         "len is strlen(FOO)");
    st_ut_eq(sizeof(FOO),   s.capacity,    "capacity is sizeof(FOO)");
    st_ut_eq(1,             s.bytes_owned, "bytes_owned is 1, memory allocated");
    st_ut_ne(NULL,          s.bytes,       "bytes is not NULL");

    ret = st_memcmp(FOO, s.bytes, s.len);
    st_ut_eq(0, ret, "s.bytes is same as FOO");
    st_ut_eq('\0', s.bytes[s.len], "s has trailing 0");

    st_str_destroy(&s);
}

st_test(str, seize) {

    {
        st_str_t s = st_str_zero;
        st_str_t t = st_str_zero;

        st_ut_eq(ST_ARG_INVALID, st_str_seize(NULL, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_seize(NULL, &s), "");
        st_ut_eq(ST_ARG_INVALID, st_str_seize(&s, NULL), "");
        st_ut_eq(ST_ARG_INVALID, st_str_seize(&s, &s), "");

        s.bytes_owned = 1;
        st_ut_bug(st_str_ref(&s, &t), "");
    }

    int ret;

    st_str_t s = st_str_zero;
    ret = st_str_init_0(&s, 10);
    st_ut_eq(ST_OK, ret, "init ok");

    st_str_t seize = st_str_zero;

    ret = st_str_seize(&seize, &s);
    st_ut_eq(ST_OK, ret, "");

    st_ut_eq(0, s.bytes_owned, "target bytes_owned does not change");

    st_ut_eq(s.type,     seize.type,        "type is same as s");
    st_ut_eq(s.len,      seize.len,         "len is same as s");
    st_ut_eq(s.capacity, seize.capacity,    "capacity is same as s");
    st_ut_eq(1,          seize.bytes_owned, "bytes_owned is 1");
    st_ut_eq(s.bytes,    seize.bytes,       "bytes is copied");

    st_str_destroy(&s);
    st_str_destroy(&seize);
}

st_test(str, increment) {
    int i_max          = INT_MAX;
    int i_value        = 0;
    uint64_t u64_max   = ULONG_MAX;
    uint64_t u64_value = 0;


    struct st_str_increment_test_s {
        st_str_t str;
        int expected_ret;
    } cases[] = {
        {
            st_str_wrap_common(&i_value, ST_TYPES_INTEGER, sizeof(i_value)),
            ST_OK,
        },
        {
            st_str_wrap_common(&i_max, ST_TYPES_INTEGER, sizeof(i_max)),
            ST_NUM_OVERFLOW,
        },
        {
            st_str_wrap_common(&u64_value, ST_TYPES_U64, sizeof(u64_value)),
            ST_OK,
        },
        {
            st_str_wrap_common(&u64_max, ST_TYPES_U64, sizeof(u64_max)),
            ST_NUM_OVERFLOW,
        },
        {
            st_str_wrap_common(NULL, ST_TYPES_NIL, 0),
            ST_TEST_EXPECT_PANIC,
        },
        /** to the following cases, the only thing matters is type */
        {
            st_str_wrap_common(&i_value, ST_TYPES_UNKNOWN, 0),
            ST_TEST_EXPECT_PANIC,
        },
        {
            st_str_wrap_common(&i_value, ST_TYPES_NIL, 0),
            ST_TEST_EXPECT_PANIC,
        },
        {
            st_str_wrap_common(&i_value, ST_TYPES_NUMBER, 0),
            ST_TEST_EXPECT_PANIC,
        },
        {
            st_str_wrap_common(&i_value, ST_TYPES_BOOLEAN, 0),
            ST_TEST_EXPECT_PANIC,
        },
        {
            st_str_wrap_common(&i_value, ST_TYPES_STRING, 0),
            ST_TEST_EXPECT_PANIC,
        },
        {
            st_str_wrap_common(&i_value, ST_TYPES_TABLE, 0),
            ST_TEST_EXPECT_PANIC,
        },
    };

    int ret;
    for (int idx = 0; idx < st_nelts(cases); idx++) {
        if (cases[idx].expected_ret == ST_TEST_EXPECT_PANIC) {
            st_ut_bug(st_str_increment(&cases[idx].str));
        }
        else {
            ret = st_str_increment(&cases[idx].str);
            st_ut_eq(ret, cases[idx].expected_ret, "wrong return value");

            if (ret != ST_OK) {
                continue;
            }

            if (cases[idx].str.type == ST_TYPES_INTEGER) {
                st_ut_eq(1, i_value, "not increment");
            }
            else {
                st_ut_eq(1, u64_value, "not increment");
            }
        }
    }

    st_ut_bug(st_str_increment(NULL));
}

/* TODO test n_arr */
st_ut_main;
