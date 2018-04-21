#include "inc/inc.h"
#include "unittest/unittest.h"

st_test(util, st_nelts)
{

    int n;

    { int arr[10];       n = st_nelts(arr); utddx(n); st_ut_eq(10, n, ""); }
    { int *arr[10];      n = st_nelts(arr); utddx(n); st_ut_eq(10, n, ""); }
    { int arr[10][10];   n = st_nelts(arr); utddx(n); st_ut_eq(10, n, ""); }
    { void *arr[10][10]; n = st_nelts(arr); utddx(n); st_ut_eq(10, n, ""); }
}

st_test(util, st_unused)
{
    int n;
    /* no warning */
    st_unused(n);
}

st_test(util, st_align)
{

    struct case_s {
        uint64_t inp;
        uint64_t upto;
        uint64_t expected;
    } cases[] = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 2, 0},
        {1, 2, 2},
        {2, 2, 2},
        {0, 4, 0},
        {1, 4, 4},
        {2, 4, 4},
        {3, 4, 4},
        {4, 4, 4},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst = st_align(c.inp, c.upto);

        utddx(rst);

        st_ut_eq(c.expected, rst, "");
    }
}

st_test(util, st_align_pow2)
{

    struct case_s {
        uint64_t inp;
        uint64_t n;
        uint64_t expected;
    } cases[] = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 2, 0},
        {0, 3, 0},
        {1, 1, 1},
        {1, 2, 2},
        {1, 3, 4},
        {1, 4, 4},
        {1, 5, 8},
        {1, 0x7fffffffffffffffULL, 0x8000000000000000ULL},
        {1, 0x8000000000000000ULL, 0x8000000000000000ULL},
        {1, 0x8000000000000001ULL, 1}, /* overflow */
        {1, 0xffffffffffffffffULL, 1}, /* overflow */
        {5, 1, 5},
        {5, 2, 6},
        {5, 3, 8},
        {5, 4, 8},
        {5, 5, 8},
        {5, 0x7fffffffffffffffULL, 0x8000000000000000ULL},
        {5, 0x8000000000000000ULL, 0x8000000000000000ULL},
        {5, 0x8000000000000001ULL, 5}, /* overflow */
        {5, 0xffffffffffffffffULL, 5}, /* overflow */
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_autotype   c = cases[i];
        st_autotype rst = st_align_pow2(c.inp, c.n);

        utddx(c.inp);
        utddx(c.n);
        utddx(rst);

        st_ut_eq(c.expected, rst);
    }
}

st_test(util, cmp_const)
{
    st_ut_eq(-1, ST_LT, "");
    st_ut_eq(0,  ST_EQ, "");
    st_ut_eq(1,  ST_GT, "");
}

st_test(util, cmp)
{

    struct case_s {
        int64_t a;
        int64_t b;
        int64_t expected;
    } cases[] = {
        {0, 0,   0},
        {1, 0,   1},
        {0, 1,   -1},
        {1, 1,   0},
        {-1, -1, 0},
        {-1, 0,  -1},
        {0, -1,  1},
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst = st_cmp(c.a, c.b);

        utddx(rst);

        st_ut_eq(c.expected, rst, "");
    }
}

st_test(util, side)
{
    st_ut_eq(0, ST_SIDE_LEFT, "");
    st_ut_eq(1, ST_SIDE_MID, "");
    st_ut_eq(1, ST_SIDE_EQ, "");
    st_ut_eq(2, ST_SIDE_RIGHT, "");

    st_ut_eq(0x10, ST_SIDE_LEFT_EQ, "");
    st_ut_eq(0x12, ST_SIDE_RIGHT_EQ, "");

    st_ut_eq(ST_SIDE_EQ, ST_SIDE_MID, "EQ is alias of MID");

    /* opposite */

    st_ut_eq(ST_SIDE_LEFT,     st_side_opposite(ST_SIDE_RIGHT), "");
    st_ut_eq(ST_SIDE_LEFT_EQ,  st_side_opposite(ST_SIDE_RIGHT_EQ), "");
    st_ut_eq(ST_SIDE_RIGHT,    st_side_opposite(ST_SIDE_LEFT), "");
    st_ut_eq(ST_SIDE_RIGHT_EQ, st_side_opposite(ST_SIDE_LEFT_EQ), "");
    st_ut_eq(ST_SIDE_MID,      st_side_opposite(ST_SIDE_MID), "");
    st_ut_eq(ST_SIDE_EQ,       st_side_opposite(ST_SIDE_EQ), "");


    /* strip */

    st_ut_eq(ST_SIDE_LEFT, st_side_strip_eq(ST_SIDE_LEFT), "");
    st_ut_eq(ST_SIDE_LEFT, st_side_strip_eq(ST_SIDE_LEFT_EQ), "");

    st_ut_eq(ST_SIDE_RIGHT, st_side_strip_eq(ST_SIDE_RIGHT), "");
    st_ut_eq(ST_SIDE_RIGHT, st_side_strip_eq(ST_SIDE_RIGHT_EQ), "");

    st_ut_eq(ST_SIDE_MID, st_side_strip_eq(ST_SIDE_MID), "");
    st_ut_eq(ST_SIDE_EQ,  st_side_strip_eq(ST_SIDE_EQ), "");

    /* if a side var has `eq` flag */

    st_ut_true(st_side_has_eq(ST_SIDE_LEFT_EQ), "");
    st_ut_true(st_side_has_eq(ST_SIDE_RIGHT_EQ), "");
    st_ut_true(st_side_has_eq(ST_SIDE_EQ), "");
    st_ut_true(st_side_has_eq(ST_SIDE_MID), "");

    st_ut_false(st_side_has_eq(ST_SIDE_LEFT), "");
    st_ut_false(st_side_has_eq(ST_SIDE_RIGHT), "");
}

st_test(util, switcher_get)
{

#define st_foomode_small 5
#define st_foomode_normal 2
#define st_foomode_extra 100

#define FOO small
#define st_foomode st_switcher_get(st_foomode_, FOO)

#if st_foomode == st_foomode_small
#else
    st_ut_fail("FOO should be small");
#endif

#if st_foomode == st_foomode_normal
    st_ut_fail("FOO should not be normal");
#endif

#if st_foomode == st_foomode_extra
    st_ut_fail("FOO should not be extra");
#endif

#if st_foomode == st_foo_xx
    st_ut_fail("FOO should not be xx");
#endif

#undef FOO
#undef st_foomode_small
#undef st_foomode_normal

}

st_test(util, bit_count_leading_zero)
{

    st_ut_eq(32 - 0,  st_bit_clz((uint32_t)0), "");
    st_ut_eq(32 - 1,  st_bit_clz((uint32_t)1), "");
    st_ut_eq(32 - 2,  st_bit_clz((uint32_t)2), "");
    st_ut_eq(32 - 2,  st_bit_clz((uint32_t)3), "");
    st_ut_eq(32 - 3,  st_bit_clz((uint32_t)4), "");
    st_ut_eq(32 - 3,  st_bit_clz((uint32_t)5), "");
    st_ut_eq(32 - 3,  st_bit_clz((uint32_t)7), "");
    st_ut_eq(32 - 4,  st_bit_clz((uint32_t)8), "");
    st_ut_eq(32 - 4,  st_bit_clz((uint32_t)9), "");
    st_ut_eq(32 - 31, st_bit_clz((uint32_t)((1ULL << 31) - 1)), "");
    st_ut_eq(32 - 32, st_bit_clz((uint32_t)((1ULL << 31) - 0)), "");

    st_ut_eq(64 - 0,  st_bit_clz((uint64_t)0), "");
    st_ut_eq(64 - 1,  st_bit_clz((uint64_t)1), "");
    st_ut_eq(64 - 2,  st_bit_clz((uint64_t)2), "");
    st_ut_eq(64 - 2,  st_bit_clz((uint64_t)3), "");
    st_ut_eq(64 - 3,  st_bit_clz((uint64_t)4), "");
    st_ut_eq(64 - 3,  st_bit_clz((uint64_t)5), "");
    st_ut_eq(64 - 3,  st_bit_clz((uint64_t)7), "");
    st_ut_eq(64 - 4,  st_bit_clz((uint64_t)8), "");
    st_ut_eq(64 - 4,  st_bit_clz((uint64_t)9), "");
    st_ut_eq(64 - 31, st_bit_clz((uint64_t)((1ULL << 31) - 1)), "");
    st_ut_eq(64 - 32, st_bit_clz((uint64_t)((1ULL << 31) + 0)), "");
    st_ut_eq(64 - 32, st_bit_clz((uint64_t)((1ULL << 31) + 1)), "");
    st_ut_eq(64 - 33, st_bit_clz((uint64_t)((1ULL << 32) + 0)), "");
    st_ut_eq(64 - 63, st_bit_clz((uint64_t)((1ULL << 63) - 1)), "");
    st_ut_eq(64 - 64, st_bit_clz((uint64_t)((1ULL << 63) + 0)), "");

    /* test one time evaluation */
    uint64_t i = 0;
    st_ut_eq(64 - 1, st_bit_clz(++i), "");

}

st_test(util, bit_most_significant_bit)
{

    st_ut_eq(-1, st_bit_msb((uint32_t)0), "");
    st_ut_eq(0,  st_bit_msb((uint32_t)1), "");
    st_ut_eq(1,  st_bit_msb((uint32_t)2), "");
    st_ut_eq(1,  st_bit_msb((uint32_t)3), "");
    st_ut_eq(2,  st_bit_msb((uint32_t)4), "");
    st_ut_eq(2,  st_bit_msb((uint32_t)5), "");
    st_ut_eq(2,  st_bit_msb((uint32_t)7), "");
    st_ut_eq(3,  st_bit_msb((uint32_t)8), "");
    st_ut_eq(3,  st_bit_msb((uint32_t)9), "");
    st_ut_eq(30, st_bit_msb((uint32_t)((1ULL << 31) - 1)), "");
    st_ut_eq(31, st_bit_msb((uint32_t)((1ULL << 31))), "");

    st_ut_eq(-1, st_bit_msb((uint64_t)0), "");
    st_ut_eq(0,  st_bit_msb((uint64_t)1), "");
    st_ut_eq(1,  st_bit_msb((uint64_t)2), "");
    st_ut_eq(1,  st_bit_msb((uint64_t)3), "");
    st_ut_eq(2,  st_bit_msb((uint64_t)4), "");
    st_ut_eq(2,  st_bit_msb((uint64_t)5), "");
    st_ut_eq(2,  st_bit_msb((uint64_t)7), "");
    st_ut_eq(3,  st_bit_msb((uint64_t)8), "");
    st_ut_eq(3,  st_bit_msb((uint64_t)9), "");
    st_ut_eq(30, st_bit_msb((uint64_t)((1ULL << 31) - 1)), "");
    st_ut_eq(31, st_bit_msb((uint64_t)((1ULL << 31))), "");
    st_ut_eq(31, st_bit_msb((uint64_t)((1ULL << 31) + 1)), "");
    st_ut_eq(32, st_bit_msb((uint64_t)((1ULL << 32))), "");
    st_ut_eq(62, st_bit_msb((uint64_t)((1ULL << 63) - 1)), "");
    st_ut_eq(63, st_bit_msb((uint64_t)((1ULL << 63) + 0)), "");

    /* test one time evaluation */
    uint64_t i = 0;
    st_ut_eq(0, st_bit_msb(++i), "");
}

st_test(util, bit_cnt1)
{

    st_ut_eq(0, st_bit_cnt1((uint32_t)0), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)1), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)2), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)4), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)8), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)(1ULL << 30)), "");
    st_ut_eq(1, st_bit_cnt1((uint32_t)(1ULL << 31)), "");

    st_ut_eq(30, st_bit_cnt1((uint32_t)((1ULL << 30) - 1)), "");
    st_ut_eq(31, st_bit_cnt1((uint32_t)((1ULL << 31) - 1)), "");
    st_ut_eq(32, st_bit_cnt1((uint32_t)(0ULL - 1)), "");


    st_ut_eq(0, st_bit_cnt1((uint64_t)0), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)1), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)2), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)4), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)8), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)(1ULL << 30)), "");
    st_ut_eq(1, st_bit_cnt1((uint64_t)(1ULL << 31)), "");

    st_ut_eq(30, st_bit_cnt1((uint64_t)((1ULL << 30) - 1)), "");
    st_ut_eq(31, st_bit_cnt1((uint64_t)((1ULL << 31) - 1)), "");

    st_ut_eq(63, st_bit_cnt1((uint64_t)((1ULL << 63) - 1)), "");
    st_ut_eq(64, st_bit_cnt1((uint64_t)(0ULL - 1)), "");
}

st_test(util, bit_cnt1_before)
{

    st_ut_eq(0, st_bit_cnt1_before((uint32_t)0ULL, -1), "");
    st_ut_eq(0, st_bit_cnt1_before((uint32_t)1ULL, -1), "");
    st_ut_eq(0, st_bit_cnt1_before((uint64_t)0ULL, -1), "");
    st_ut_eq(0, st_bit_cnt1_before((uint64_t)1ULL, -1), "");

    st_ut_eq(0, st_bit_cnt1_before((uint32_t)0ULL, 0), "");
    st_ut_eq(0, st_bit_cnt1_before((uint32_t)1ULL, 0), "");
    st_ut_eq(1, st_bit_cnt1_before((uint32_t)1ULL, 1), "");
    st_ut_eq(1, st_bit_cnt1_before((uint32_t)3ULL, 1), "");
    st_ut_eq(2, st_bit_cnt1_before((uint32_t)3ULL, 2), "");

    st_ut_eq(0, st_bit_cnt1_before((uint32_t)(0ULL - 1), 0), "");
    st_ut_eq(1, st_bit_cnt1_before((uint32_t)(0ULL - 1), 1), "");
    st_ut_eq(2, st_bit_cnt1_before((uint32_t)(0ULL - 1), 2), "");
    st_ut_eq(31, st_bit_cnt1_before((uint32_t)(0ULL - 1), 31), "");
    st_ut_eq(32, st_bit_cnt1_before((uint32_t)(0ULL - 1), 32), "");
    st_ut_eq(32, st_bit_cnt1_before((uint32_t)(0ULL - 1), 33), "");


    st_ut_eq(0,  st_bit_cnt1_before((uint64_t)(0ULL - 1), 0), "");
    st_ut_eq(1,  st_bit_cnt1_before((uint64_t)(0ULL - 1), 1), "");
    st_ut_eq(2,  st_bit_cnt1_before((uint64_t)(0ULL - 1), 2), "");
    st_ut_eq(31, st_bit_cnt1_before((uint64_t)(0ULL - 1), 31), "");
    st_ut_eq(32, st_bit_cnt1_before((uint64_t)(0ULL - 1), 32), "");
    st_ut_eq(33, st_bit_cnt1_before((uint64_t)(0ULL - 1), 33), "");
    st_ut_eq(63, st_bit_cnt1_before((uint64_t)(0ULL - 1), 63), "");
    st_ut_eq(64, st_bit_cnt1_before((uint64_t)(0ULL - 1), 64), "");
    st_ut_eq(64, st_bit_cnt1_before((uint64_t)(0ULL - 1), 65), "");
}

/*
 * TODO
 *     offset
 *     by_offset
 *     owner
 *     min
 *     max
 *     foreach
 */

st_test(util, bug_handler)
{
    st_util_bughandler_t f = st_util_bughandler_raise;
    st_ut_eq((void*)f, (void*)st_util_bughandler_);

    st_ut_bug(st_bug(), "st_bug() should make a bug");
    st_ut_nobug((void)1, "(void)1 should not make a bug");
}

st_ut_main;
