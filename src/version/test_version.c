#include "version.h"
#include "unittest/unittest.h"


#define st_ut_eq_version(cptr, field)           \
    st_ut_eq(cptr->field,                       \
             field,                             \
             "wrong " #field " number: %s, %d", \
             cptr->ver_str,                     \
             field)


int
st_version_relation(const char *v1,
                    const char *v2,
                    st_version_relation_t *relation);

int st_version_parse(const char *ver_str, int *major, int *minor, int *release);


st_test(version, compatible)
{
    struct st_version_test_compatible_s {
        char *ver_str;
        int is_compatible;
        int ret;
        int relation;
    };

    char *v1 = "1.0.0";
    char *v2 = "1.0.0";

    int ret = st_version_is_compatible(NULL, v2);
    st_ut_eq(0, ret, "v1 is NULL");

    ret = st_version_is_compatible(v1, NULL);
    st_ut_eq(0, ret, "v2 is NULL");

    struct st_version_test_compatible_s cases[] = {
        { "1.0.0", 1, ST_OK, ST_VERSION_THE_SAME   },
        { "1.1.0", 1, ST_OK, ST_VERSION_COMPATIBLE },
        { "1.0.1", 1, ST_OK, ST_VERSION_COMPATIBLE },
        { "2.0.0", 0, ST_OK, ST_VERSION_CONFLICT   }
    };

    for (int idx = 0; idx < st_nelts(cases); idx++) {
        struct st_version_test_compatible_s *cptr = &cases[idx];

        st_version_relation_t relation = -1;
        int ret = st_version_relation(v1, cptr->ver_str, &relation);
        st_ut_eq(cptr->ret, ret, "failed: %s", cptr->ver_str);
        st_ut_eq(cptr->relation, relation, "failed: %s", cptr->ver_str);

        ret = st_version_is_compatible(v1, cptr->ver_str);
        st_ut_eq(cptr->is_compatible, ret, "failed: %s", cptr->ver_str);
    }
}


st_test(version, parse)
{
    struct st_version_test_case_s {
        char *ver_str;
        int expected_ret;
        int major;
        int minor;
        int release;
    };

    int major   = -1;
    int minor   = -1;
    int release = -1;
    int ret     = ST_OK;

    struct st_version_test_case_s cases[] = {
        { NULL,          ST_ARG_INVALID, -1,   -1,   -1   },
        { "",            ST_ARG_INVALID, -1,   -1,   -1   },
        { "1",           ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.",          ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.2",         ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.2.",        ST_ARG_INVALID, -1,   -1,   -1   },
        { ".",           ST_ARG_INVALID, -1,   -1,   -1   },
        { "1..",         ST_ARG_INVALID, -1,   -1,   -1   },
        { "a",           ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.a.3",       ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.2.a",       ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.2.3.4",     ST_ARG_INVALID, -1,   -1,   -1   },
        { "1.2.3",       ST_OK,           1,    2,    3   },
        { "111.222.333", ST_OK,           111,  222,  333 },
    };

    for (int idx = 0; idx < st_nelts(cases); idx++) {
        struct st_version_test_case_s *cptr = &cases[idx];
        ret = st_version_parse(cptr->ver_str, &major, &minor, &release);
        st_ut_eq(cptr->expected_ret, ret, "return value is wrong");

        if (ret == ST_OK) {
            st_ut_eq_version(cptr, major);
            st_ut_eq_version(cptr, minor);
            st_ut_eq_version(cptr, release);
        }
    }
}

st_ut_main;
