#ifndef __INC__LOG_H__
#     define __INC__LOG_H__

#include <stdio.h>

#define st_stderr_printf( _fmt, ...)  fprintf( stderr, _fmt, ##__VA_ARGS__)
#define st_log_printf( _fmt, _lvl, ...)                                       \
        st_stderr_printf( "%16s %4d %24s %7s " _fmt,                          \
                           __FILE__, __LINE__, __func__, _lvl, ##__VA_ARGS__)

#ifdef ST_DEBUG
#   define dd( _fmt, ... ) st_log_printf(  _fmt "\n", "[DEBUG]", ##__VA_ARGS__ )
#   define dlog( _fmt, ... ) st_log_printf( _fmt, "[DEBUG]", ##__VA_ARGS__ )
#   define ddv( _fmt, ... ) st_stderr_printf( _fmt, ##__VA_ARGS__ )
#else
#   define dd( _fmt, ... )
#   define dlog( _fmt, ... )
#   define ddv( _fmt, ... )
#endif /* ST_DEBUG */

#define dinfo( _fmt, ... ) st_log_printf( _fmt "\n",  "[INFO] ", ##__VA_ARGS__ )
#define dwarn( _fmt, ... ) st_log_printf( _fmt "\n",  "[WARN] ", ##__VA_ARGS__ )
#define derr( _fmt, ... ) st_log_printf(  _fmt "\n", "[ERROR] ", ##__VA_ARGS__ )
#define derrno( _fmt, ... ) derr( _fmt " errno: %s", ##__VA_ARGS__, \
                                  strerror( errno ) )

#if 1

#define st_malloc malloc
#define st_free free

#else
    static inline void*
st_malloc( size_t size )
{
    void *r;
    r = malloc( size );
    dd( "MALLOC: %p", r );
    return r;
}

    static inline void
st_free( void* pnt )
{
    dd( "FREE: %p", pnt );
    free( pnt );
}
#endif



#endif /* __INC__LOG_H__ */
