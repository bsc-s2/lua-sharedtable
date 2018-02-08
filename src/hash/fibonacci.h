#ifndef HASH__FIBONACCI_H
#define HASH__FIBONACCI_H

#include <stdint.h>

/*
 *          √5 - 1
 * golden = ------- =~ 0.618
 *             2
 */


static inline uint16_t
fib_hash16(uint16_t i) {
    /* golden * (1<<16) */
    return i * 40503;
}

static inline uint32_t
fib_hash32(uint32_t i) {
    /* golden * (1<<32) */
    return i * 2654435769;
}

static inline uint64_t
fib_hash64(uint64_t i) {
    /* golden * (1<<64) */
    return i * 11400714819323198485ull;
}

static inline uint8_t
fib_hash8(uint8_t i) {
    return (uint8_t)fib_hash16(i);
}

#endif /* HASH__FIBONACCI_H */
