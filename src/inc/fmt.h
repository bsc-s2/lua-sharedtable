#ifndef inc_fmt_
#define inc_fmt_

#include <inttypes.h>

#define st_fmt_wrap(a, v, b) _Generic((v),                                    \
                 char      : a   "%c" b,                                      \
        signed   char      : a   "%c" b,                                      \
                 short     : a  "%hd" b,                                      \
                 int       : a   "%d" b,                                      \
                 long      : a  "%ld" b,                                      \
                 long long : a "%lld" b,                                      \
        unsigned char      : a   "%u" b,                                      \
        unsigned short     : a  "%hu" b,                                      \
        unsigned int       : a   "%u" b,                                      \
        unsigned long      : a  "%lu" b,                                      \
        unsigned long long : a "%llu" b,                                      \
                 float     : a   "%f" b,                                      \
                 double    : a   "%f" b,                                      \
                 void*     : a   "%p" b,                                      \
                 default   : a   "%p" b                                       \
)


/* NOTE:
 * In c standards: char, signed char and unsidned char are 3 distinct types!
 */

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
