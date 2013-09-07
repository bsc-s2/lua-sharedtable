#ifndef __HASH__FIB_HASH_H__
#     define __HASH__FIB_HASH_H__

#include <stdint.h>


    static inline uint16_t
fib_hash16( uint16_t i )
{
    return i * 40503;
}

    static inline uint32_t
fib_hash32( uint32_t i )
{
    return i * 2654435769;
}

    static inline uint64_t
fib_hash64( uint64_t i )
{
    return i * 11400714819323198485ull;
}

    static inline uint8_t
fib_hash8( uint8_t i )
{
    return ( uint8_t )fib_hash16( i );
}

#endif /* __HASH__FIB_HASH_H__ */
