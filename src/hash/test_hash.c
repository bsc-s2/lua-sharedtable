#include <stdint.h>
#include <stdio.h>

#include "unittest/unittest.h"
#include "crc32.h"
#include "fibonacci.h"


enum fib_size_e {
    fib8,
    fib16,
    fib32,
    fib64,
};


int get_fib_variance(enum fib_size_e fib_size)
{

    int n = 1024 * 1024 * 10;
    int n_bucket = 256;
    int buckets[n_bucket];
    int i;
    uint64_t hash;

    for (i = 0; i < n_bucket; i++) {
        buckets[i] = 0;
    }

    for (i = 0; i < n; i++) {
        switch (fib_size) {
            case fib8:
                hash = fib_hash8(i) % n_bucket;
                break;
            case fib16:
                hash = fib_hash16(i) % n_bucket;
                break;
            case fib32:
                hash = fib_hash32(i) % n_bucket;
                break;
            case fib64:
                hash = fib_hash64(i) % n_bucket;
                break;
        }
        buckets[(int)hash] += 1;
    }

    int max = 0;
    int min = n;

    for (i = 0; i < n_bucket; i++) {
        if (buckets[i] > max) {
            max = buckets[i];
        }
        if (buckets[i] < min) {
            min = buckets[i];
        }
    }

    return (max - min) * 1000 / (n / n_bucket);
}

st_test(hash, fib_64)
{

    int variance;

    variance = get_fib_variance(fib8);
    st_ut_le(variance, 3, "variance of fib 8 should be smaller than 3‰");

    variance = get_fib_variance(fib16);
    st_ut_le(variance, 3, "variance of fib 16 should be smaller than 3‰");

    variance = get_fib_variance(fib32);
    st_ut_le(variance, 3, "variance of fib 32 should be smaller than 3‰");

    variance = get_fib_variance(fib64);
    st_ut_le(variance, 3, "variance of fib 64 should be smaller than 3‰");
}


st_test(hash, crc32)
{

    struct case_s {
        char    *inp;
        int      len;
        uint32_t expected;
    } cases[] = {
        { "",    0, 0 },
        { "a",   1, 3904355907 },
        { "1",   1, 2212294583 },
        { "123", 3, 2286445522 },
        {
            "\0\1\2\3fjkldsfjdkaslfdjsaklfdjsaklfd;sfjklsajfdkls"
            ";afjdklsafjdslka;fdjsalkf;dsjaklfd;sadfkl", 88, 1243232475
        },
    };

    for (int i = 0; i < st_nelts(cases); i++) {
        st_typeof(cases[0])   c = cases[i];
        st_typeof(c.expected) rst = crc32((uint8_t *)c.inp, c.len);

        ddx(rst);

        st_ut_eq(c.expected, rst, "");
    }
}

/* TODO test murmur */

st_ut_main;
