#ifndef inc_fmt_
#define inc_fmt_

#include <inttypes.h>

#define st_fmt_wrap(a, v, b) _Generic((v),                                    \
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
    default:   a "%p"      b                                                  \
)


/* In c standards: char, signed char and unsidned char are 3 distinct types! */

#define st_fmt_map_                                                           \
                 char      :   "%c",                                          \
        signed   char      :   "%c",                                          \
                 short     :  "%hd",                                          \
                 int       :   "%d",                                          \
                 long      :  "%ld",                                          \
                 long long : "%lld",                                          \
        unsigned char      :   "%u",                                          \
        unsigned short     :  "%hu",                                          \
        unsigned int       :   "%u",                                          \
        unsigned long      :  "%lu",                                          \
        unsigned long long : "%llu",                                          \
                 float     :   "%f",                                          \
                 double    :   "%f",                                          \
                 void*     :   "%p"

#define st_fmt_of(v) _Generic((v), st_fmt_map_, default: "%p")

#define st_fmt_strict_of(v) _Generic((v), st_fmt_map_ )


#endif /* inc_fmt_ */
