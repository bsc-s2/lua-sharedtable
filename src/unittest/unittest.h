#ifndef __UNITTEST__UNITTEST_H__
#define __UNITTEST__UNITTEST_H__

#include <errno.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <inttypes.h>

#include "inc/inc.h"

#ifdef ST_DEBUG_UNITTEST
#   define OK_OUT( _fmt, ... ) st_log_printf( _fmt "\n", "[TEST]  OK", ##__VA_ARGS__ )
#   define st_ut_bench_debug( _fmt, ... ) st_log_printf( _fmt "\n", "[BENCH]", ##__VA_ARGS__ )
#   define st_ut_debug( _fmt, ... ) st_log_printf( _fmt "\n", "[UT-DEBUG]", ##__VA_ARGS__ )
#else
#   define OK_OUT( ... )
#   define st_ut_bench_debug( _fmt, ... )
#   define st_ut_debug( _fmt, ... )
#endif /* ST_DEBUG_UNITTEST */

#define ERR_OUT( _fmt, ... ) st_log_printf( _fmt "\n", "[TEST] ERR", ##__VA_ARGS__ )

#define BENCH_INFO( _fmt, ... ) st_log_printf( _fmt "\n", "[BENCH]", ##__VA_ARGS__ )

#define st_pointer_t "%p"
#define st_pointer void*

typedef struct st_ut_case_t st_ut_case_t;
typedef struct st_ut_bench_t st_ut_bench_t;

extern st_ut_case_t *st_ut_cases_;
extern int64_t       st_ut_case_n_;
extern int64_t       st_ut_case_capacity_;
extern jmp_buf       st_ut_bug_jmp_buf_;
extern jmp_buf       st_ut_case_jmp_buf_;

extern st_ut_bench_t *st_ut_benches_;
extern int64_t        st_ut_bench_n_;
extern int64_t        st_ut_bench_capacity_;

typedef void (*st_ut_case_f)();

typedef void (*st_ut_bench_f)(void *data, int64_t n);
typedef void *(*st_ut_bench_init_f)();
typedef void (*st_ut_bench_clean_f)(void *data);

static inline int main_(int argc, char **argv);

static inline void st_ut_add_case(char *fname, st_ut_case_f func);

static inline int st_ut_is_bench(int argc, char **argv);
static inline void st_ut_add_bench(char *fname,
                                    st_ut_bench_f func,
                                    st_ut_bench_init_f init,
                                    st_ut_bench_clean_f clean,
                                    int64_t n_sec);
static inline void
st_ut_run_bench(st_ut_bench_t *b);


char *st_ut_get_tostring_buf();
void st_ut_bughandler_track();


/* define test */
#define st_test(case_, func_)                                                 \
    static void st_ut_case_##case_##_##func_();                               \
    __attribute__((constructor))                                              \
    void st_ut_case_##case_##_##func_##_constructor_() {                      \
        st_ut_add_case(#case_ "::" #func_, st_ut_case_##case_##_##func_);     \
    }                                                                         \
    static void st_ut_case_##case_##_##func_()

/* define benchmark */
#define st_ben(name_, func_, n_sec, times) \
        st_bench(name_, func_, NULL, NULL, n_sec, data, times)

#define st_bench(name_, func_, init_func, clean_func, n_sec, data, times)     \
    static void st_ut_bench_##name_##_##func_(void*data, int64_t times);      \
    __attribute__((constructor))                                              \
    void st_ut_bench_##name_##_##func_##_constructor_() {                     \
        st_ut_add_bench(#name_ "::" #func_, st_ut_bench_##name_##_##func_,    \
                         init_func, clean_func, n_sec);                       \
    }                                                                         \
    static void st_ut_bench_##name_##_##func_(void*data, int64_t times)


/* assertions */
#define st_ut_true(actual, fmt...)         st_ut_assert_cmp_(0,        !=, actual, fmt)
#define st_ut_false(actual, fmt...)        st_ut_assert_cmp_(0,        ==, actual, fmt)
#define st_ut_eq(expected, actual, fmt...) st_ut_assert_cmp_(expected, ==, actual, fmt)
#define st_ut_ne(expected, actual, fmt...) st_ut_assert_cmp_(expected, !=, actual, fmt)
#define st_ut_ge(expected, actual, fmt...) st_ut_assert_cmp_(expected, >=, actual, fmt)
#define st_ut_gt(expected, actual, fmt...) st_ut_assert_cmp_(expected, >,  actual, fmt)
#define st_ut_le(expected, actual, fmt...) st_ut_assert_cmp_(expected, <=, actual, fmt)
#define st_ut_lt(expected, actual, fmt...) st_ut_assert_cmp_(expected, <,  actual, fmt)
#define st_ut_succeed(fmt...)              st_ut_assert_(1, fmt)
#define st_ut_fail(fmt...)                 st_ut_assert_(0, fmt)

#define st_ut_bug(make_bug_, mes...)                                          \
        do {                                                                  \
            st_util_bughandler_t old = st_util_bughandler_;                   \
            st_util_bughandler_ = st_ut_bughandler_track;                     \
                                                                              \
            int r_ = setjmp(st_ut_bug_jmp_buf_);                              \
            if (r_ == 0) {                                                    \
                /* after set jmp */                                           \
                make_bug_;                                                    \
                st_util_bughandler_ = old;                                    \
                st_ut_fail("Expect but NO. " mes);                            \
            } else {                                                          \
                /* return from longjmp */                                     \
                st_util_bughandler_ = old;                                    \
                st_ut_succeed("Expected bug found. " mes);                    \
            }                                                                 \
                                                                              \
        } while (0)

#define st_ut_nobug(make_nobug_, mes...)                                      \
        do {                                                                  \
            st_util_bughandler_t old = st_util_bughandler_;                   \
            st_util_bughandler_ = st_ut_bughandler_track;                     \
                                                                              \
            int r_ = setjmp(st_ut_bug_jmp_buf_);                              \
            if (r_ == 0) {                                                    \
                /* after set jmp */                                           \
                make_nobug_;                                                  \
                st_util_bughandler_ = old;                                    \
                st_ut_succeed("Expected no bug. " mes);                       \
            } else {                                                          \
                /* return from longjmp */                                     \
                st_util_bughandler_ = old;                                    \
                st_ut_fail("Expected no bug but found one. " mes);            \
            }                                                                 \
        } while (0)

#define st_ut_assert_cmp_(expected__, operator__, actual__, fmt, ...)         \
    do {                                                                      \
        st_typeof(expected__) e__ = (expected__);                             \
        st_typeof(actual__)   a__ = (actual__);                               \
        int rst_ = st_ut_compare(e__, a__);                                   \
        st_ut_assert_(rst_ operator__ 0,                                      \
             "Expected: '%s' " #operator__ " '%s'; " fmt,                     \
             st_ut_printf_(e__),                                              \
             st_ut_printf_(a__),                                              \
             ##__VA_ARGS__ );                                                 \
    } while (0)

#define st_ut_assert_( to_be_true, mes, ... )                                 \
    do {                                                                      \
        if ( (to_be_true) ) {                                                 \
            OK_OUT( mes, ##__VA_ARGS__ );                                     \
        }                                                                     \
        else {                                                                \
            ERR_OUT( mes, ##__VA_ARGS__ );                                    \
            longjmp(st_ut_case_jmp_buf_, 1);                                  \
        }                                                                     \
    } while ( 0 )

struct st_ut_case_t {
    char *name;
    st_ut_case_f func;
};
struct st_ut_bench_t {
    char *name;
    int64_t n_sec;
    st_ut_bench_f func;
    st_ut_bench_init_f init;
    st_ut_bench_clean_f clean;
};

static inline void
st_ut_run_bench(st_ut_bench_t *b) {
    int            rc;
    struct timeval t0;
    struct timeval t1;
    int64_t        spent_usec;
    void          *data;

    for (int64_t n = 64; n > 0 /* overflow */; ) {
        data = NULL;
        if (b->init != NULL) {
            data = b->init(n);
        }

        gettimeofday(&t0, NULL);

        printf("benching %s n=%lld:\n", b->name, n);
        b->func(data, n);

        gettimeofday(&t1, NULL);

        if (b->clean != NULL) {
            b->clean(data);
        }
        spent_usec = (t1.tv_sec - t0.tv_sec);
        spent_usec = spent_usec*1000*1000 + (t1.tv_usec - t0.tv_usec);

        st_ut_bench_debug("%s cost: %ld ms, n=%llu", b->name, spent_usec/1000, n);
        if (spent_usec > b->n_sec * 1000 * 1000) {
            double ns_per_call = (double)spent_usec * 1000 / n;
            BENCH_INFO(
                "%s runs"
                "    %llu times\n"
                "    costs %lld mseconds\n"
                "    calls/s: %llu\n"
                "    ns/call(10e-9): %.3f",
                b->name, n, spent_usec / 1000,
                n * 1000 * 1000 / spent_usec, ns_per_call);
            break;
        } else {
            if (spent_usec > 10*1000) {
                n = b->n_sec * 1000 * 1000 / spent_usec * n * 3/2;
            } else {
                n = n * 4;
            }
        }
    }
}

#define st_ut_compare(a, b) st_ut_norm_cmp(a, b)


/* force type conversion */
#define st_ut_norm_cmp(a, b) st_ut_norm_cmp_(a, (st_typeof(a))b)
#define st_ut_norm_cmp_(a, b) ((a)>(b) ? 1 : (((a)<(b)) ? -1 : 0))

static inline int st_cmp_void_ptr(void  *a, void *b) { return st_ut_norm_cmp(a, b); }

#define st_ut_printf_(v) ({                                                   \
        char *buf__ = st_ut_get_tostring_buf();                               \
        snprintf(buf__, 64, st_fmt_of(v), v);                                 \
        buf__;                                                                \
    })


static inline int st_ut_is_bench(int argc, char **argv) {
    return (argc >= 2)
           && (sizeof("bench") - 1 == strlen(argv[1]))
           && (0 == memcmp(argv[1], "bench", sizeof("bench") - 1));
}

static inline void st_ut_add_case(char *fname, st_ut_case_f func) {
    if (st_ut_cases_ == NULL) {
        st_ut_case_capacity_ = 1;
        st_ut_cases_ = malloc(sizeof(*st_ut_cases_) * st_ut_case_capacity_);
        st_ut_case_n_ = 0;
    }

    while (st_ut_case_n_ >= st_ut_case_capacity_) {
        st_ut_case_capacity_ *= 4;
        st_ut_cases_ = realloc(st_ut_cases_,
                                sizeof(*st_ut_cases_) * st_ut_case_capacity_);
    }

    st_ut_cases_[st_ut_case_n_].name = fname;
    st_ut_cases_[st_ut_case_n_].func = func;
    st_ut_case_n_++;
}

static inline void st_ut_add_bench(char *fname,
                                    st_ut_bench_f func,
                                    st_ut_bench_init_f init,
                                    st_ut_bench_clean_f clean,
                                    int64_t n_sec) {
    if (st_ut_benches_ == NULL) {
        st_ut_bench_capacity_ = 1;
        st_ut_benches_ = malloc(sizeof(*st_ut_benches_) * st_ut_bench_capacity_);
        st_ut_bench_n_ = 0;
    }

    while (st_ut_bench_n_ >= st_ut_bench_capacity_) {
        st_ut_bench_capacity_ *= 4;
        st_ut_benches_ = realloc(st_ut_benches_,
                                  sizeof(*st_ut_benches_) * st_ut_bench_capacity_);
    }

    st_ut_bench_t *b = &st_ut_benches_[st_ut_bench_n_];
    b->name = fname;
    b->func = func;
    b->init = init;
    b->clean = clean;
    b->n_sec = n_sec ? : 1;
    st_ut_bench_n_++;
}

static inline int main_(int argc, char **argv) {

    /* test */
    for (int i = 0; i < st_ut_case_n_; i++) {
        printf("### Test Start: %d %s\n", i, st_ut_cases_[i].name);

        int r = setjmp(st_ut_case_jmp_buf_);
        if (r == 0) {
            st_ut_cases_[i].func();
            printf("### Test OK:    %d %s\n", i, st_ut_cases_[i].name);
        } else {
            /* longjmp triggered, there is a failure */
            printf("### Test Fail:  %d %s\n", i, st_ut_cases_[i].name);
            return -1;
        }
    }

    printf("All passed, n_cases = %lld\n", st_ut_case_n_);

    /* bench */
    if (st_ut_is_bench(argc, argv)) {

        for (int i = 0; i < st_ut_bench_n_; i++) {

            st_ut_bench_t *b = &st_ut_benches_[i];
            printf("bench: %s\n", b->name);
            st_ut_run_bench(b);
        }
        printf("benches done:\n");
    }

    return 0;
}

#define st_ut_main                                                            \
    st_ut_case_t  *st_ut_cases_;                                              \
    int64_t        st_ut_case_n_;                                             \
    int64_t        st_ut_case_capacity_;                                      \
    jmp_buf        st_ut_bug_jmp_buf_;                                        \
    jmp_buf        st_ut_case_jmp_buf_;                                       \
    st_ut_bench_t *st_ut_benches_;                                            \
    int64_t        st_ut_bench_n_;                                            \
    int64_t        st_ut_bench_capacity_;                                     \
                                                                              \
    char *st_ut_get_tostring_buf() {                                          \
        static __thread char buffers[128][64];                                \
        static __thread uint64_t i = 0;                                       \
        char *buffer = buffers[i++ % 128];                                    \
        return buffer;                                                        \
    }                                                                         \
                                                                              \
    void st_ut_bughandler_track() {                                           \
        longjmp(st_ut_bug_jmp_buf_, 1);                                       \
    }                                                                         \
                                                                              \
    int main(int argc, char **argv) {                                         \
        return main_(argc, argv);                                             \
    }

#endif /* __UNITTEST__UNITTEST_H__ */
