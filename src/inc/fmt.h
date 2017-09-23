#ifndef inc_fmt_
#define inc_fmt_

#include <inttypes.h>

#define st_fmt_of(a, v, b) _Generic((v),                                      \
    char:      a "%c"      b ,                                                \
    int8_t:    a "%"PRIi8  b ,                                                \
    int16_t:   a "%"PRIi16 b ,                                                \
    int32_t:   a "%"PRIi32 b ,                                                \
    int64_t:   a "%"PRIi64 b ,                                                \
    uint8_t:   a "%"PRIu8  b ,                                                \
    uint16_t:  a "%"PRIu16 b ,                                                \
    uint32_t:  a "%"PRIu32 b ,                                                \
    uint64_t:  a "%"PRIu64 b ,                                                \
    void*:     a "%p"      b ,                                                \
    default:   a "%x"      b                                                  \
)


#endif /* inc_fmt_ */
