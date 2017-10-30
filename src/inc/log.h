#ifndef __INC__LOG_H__
#     define __INC__LOG_H__

#include <stdio.h>

#define st_stderr_printf( _fmt, ...)  fprintf( stderr, _fmt, ##__VA_ARGS__)
#define st_log_printf( _fmt, _lvl, ...)                                       \
        st_stderr_printf( "%16s %4d %24s %7s " _fmt,                          \
                           __FILE__, __LINE__, __func__, _lvl, ##__VA_ARGS__)

#ifdef ST_DEBUG
#   define dd( _fmt, ... )   st_log_printf(_fmt "\n", "[DEBUG]", ##__VA_ARGS__)
#   define dlog( _fmt, ... ) st_log_printf(_fmt, "[DEBUG]", ##__VA_ARGS__)
#   define ddv( _fmt, ... )  st_stderr_printf( _fmt, ##__VA_ARGS__)
#   define ddx(n)            st_stderr_printf(st_fmt_of(#n "=", n, "\n"), n)
#else
#   define dd( _fmt, ... )
#   define dlog( _fmt, ... )
#   define ddv( _fmt, ... )
#   define ddx(n)
#endif /* ST_DEBUG */

#define dinfo( _fmt, ... ) st_log_printf( _fmt "\n",  "[INFO] ", ##__VA_ARGS__ )
#define dwarn( _fmt, ... ) st_log_printf( _fmt "\n",  "[WARN] ", ##__VA_ARGS__ )
#define derr( _fmt, ... ) st_log_printf(  _fmt "\n", "[ERROR] ", ##__VA_ARGS__ )
#define derrno( _fmt, ... ) derr( _fmt " errno: %s", ##__VA_ARGS__, \
                                  strerror( errno ) )

#endif /* __INC__LOG_H__ */
