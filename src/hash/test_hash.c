#include <stdint.h>
#include <stdio.h>

#include "crc32.h"
#include "fibonacci.h"

int
test_crc32(uint8_t *buf, int len, uint32_t expected) {
    uint32_t crc;
    crc = crc32(buf, len);

    if (crc != expected) {
        printf("Wrong crc for '%.*s', expected: %u, actual: %u\n",
               len, buf, expected, crc);
        return -1;
    }
    return 0;
}

int
test_fib() {
    uint64_t i;
    for (i = 0; i < 1024 * 1024; i++) {
        fib_hash8((uint8_t)i);
        fib_hash16((uint16_t)i);
        fib_hash32((uint32_t)i);
        fib_hash64((uint64_t)i);
    }

    return 0;
}

int
main(int argc, char **argv) {
    test_crc32((uint8_t *)"", 0, 0);
    test_crc32((uint8_t *)"a", 1, 3904355907);
    test_crc32((uint8_t *)"1", 1, 2212294583);
    test_crc32((uint8_t *)"123", 3, 2286445522);
    test_crc32((uint8_t *)
               "\0\1\2\3fjkldsfjdkaslfdjsaklfdjsaklfd;sfjklsajfdkls;afjdklsafjdslka;fdjsalkf;dsjaklfd;sadfkl", 88,
               1243232475);

    test_fib();
    return 0;
}
