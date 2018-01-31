#ifndef _TIME_H_INCLUDED_
#define _TIME_H_INCLUDED_

#include <sys/time.h>
#include <errno.h>
#include "inc/inc.h"

static inline int64_t st_time_in_usec(int64_t *usec) {

    struct timeval tm;

    if (gettimeofday(&tm, NULL) != 0) {
        return errno;
    }

    *usec = tm.tv_sec * 1000000 + tm.tv_usec;

    return ST_OK;
}

#endif /* _GC_H_INCLUDED_ */
