#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "version.h"
#include "inc/inc.h"


#ifndef ST_VERSION_FULL
#define ST_VERSION_FULL "0.0.0"
#endif


static const char *st_version = ST_VERSION_FULL;


static int
st_version_atoi(const char *nptr, int *val, char **endptr)
{
    st_must(nptr != NULL, ST_ARG_INVALID);

    errno = 0;
    *endptr = NULL;

    long int num = strtol(nptr, endptr, 10);
    if (errno) {
        return errno;
    }


    if (nptr == *endptr) {
        return ST_ARG_INVALID;
    }

    if (num > INT_MAX || num < INT_MIN) {
        return ST_NUM_OVERFLOW;
    }

    *val = num;

    return ST_OK;
}


int
st_version_parse(const char *ver_str, int *major, int *minor, int *release)
{
    st_must(ver_str != NULL, ST_ARG_INVALID);
    st_must(major != NULL, ST_ARG_INVALID);
    st_must(minor != NULL, ST_ARG_INVALID);
    st_must(release != NULL, ST_ARG_INVALID);

    int vers[3];

    int num = 2;
    char *endptr = NULL;
    const char *vptr = ver_str;
    for (int idx = 0; idx <= num; idx++) {
        int ret = st_version_atoi(vptr, &vers[idx], &endptr);
        if (ret != ST_OK) {
            return ret;
        }

        if (idx != num && *endptr != '.') {
            return ST_ARG_INVALID;
        }

        if (idx == num && *endptr != '\0') {
            return ST_ARG_INVALID;
        }

        vptr = endptr + 1;
    }

    *major   = vers[0];
    *minor   = vers[1];
    *release = vers[2];

    return ST_OK;
}


int
st_version_relation(const char *v1,
                    const char *v2,
                    st_version_relation_t *relation)
{
    int major1, minor1, release1;
    int major2, minor2, release2;

    int ret = st_version_parse(v1, &major1, &minor1, &release1);
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_version_parse(v2, &major2, &minor2, &release2);
    if (ret != ST_OK) {
        return ret;
    }

    if (major1 != major2) {
        *relation = ST_VERSION_CONFLICT;
        goto out;
    }

    *relation = ST_VERSION_COMPATIBLE;
    if (minor1 != minor2) {
        goto out;
    }

    if (release1 == release2) {
        *relation = ST_VERSION_THE_SAME;
    }

out:
    return ST_OK;
}


int
st_version_is_compatible(const char *v1, const char *v2)
{
    st_must(v1 != NULL, 0);
    st_must(v2 != NULL, 0);

    st_version_relation_t relation;
    int ret = st_version_relation(v1, v2, &relation);
    if (ret != ST_OK) {
        return 0;
    }

    return (relation != ST_VERSION_CONFLICT);
}


const char *
st_version_get_fully(void)
{
    return st_version;
}
