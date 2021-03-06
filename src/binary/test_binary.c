#include <stdio.h>

#include "inc/inc.h"
#include "binary.h"
#include "unittest/unittest.h"

void test_bin_varint(uint64_t i) {

    int      r;
    int      rbuflen;
    int      buflen;
    uint64_t j;
    uint8_t  buf[10];

    /* test size == len-of-put */

    rbuflen = bin_vu64_size(i);

    r = bin_vu64_put(buf, 10, i);
    st_ut_ge(r, 0, "i=%"PRIu64, i);
    st_ut_eq(rbuflen, r, "i=%"PRIu64, i);

    /* test buf error */

    buflen = r;

    r = bin_vu64_put(buf, buflen - 1, i);
    st_ut_eq(ST_BUF_OVERFLOW, r, "i=%"PRIu64, i);

    r = bin_vu64_load(buf, buflen - 1, &j);
    st_ut_eq(ST_BUF_NOT_ENOUGH, r, "i=%"PRIu64, i);

    /* test load */

    rbuflen = bin_vu64_load(buf, 10, &j);
    st_ut_gt(rbuflen, 0, "i=%"PRIu64, i);

    st_ut_eq(i, j, "i=%"PRIu64, i);
    st_ut_eq(buflen, rbuflen, "i=%"PRIu64, i);
}

st_test(binary, varint) {
    for (uint64_t i = 0; i < 1024; i++) {
        test_bin_varint(i);
    }
}

st_test(binary, varint_edge) {

    for (int64_t i = 0; i < 64; i++) {

        test_bin_varint((1llu << i) + 1);
        test_bin_varint((1llu << i));
        test_bin_varint((1llu << i) - 1);
    }
}

st_bench(binary, vu64_size, NULL, NULL, 2, data, n) {
    for (int64_t i = 0; i < n; i++) {
        bin_vu64_size(103294);
    }
}

st_bench(binary, vu64_put, NULL, NULL, 2, data, n) {
    uint8_t buf[10];
    for (int64_t i = 0; i < n; i++) {
        bin_vu64_put(buf, 10, (1ULL << 60) + i);
    }
}

st_ut_main;
