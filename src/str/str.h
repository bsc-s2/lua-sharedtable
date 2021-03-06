#ifndef ST_STR_
#define ST_STR_

#include <string.h>
#include <limits.h>

#include "inc/inc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_str_s st_str_t;
struct st_str_s {

    union{
        /**
         * user defined type info.
         *
         * Notice:
         *     0 must mean str
         */
        int64_t      type;
        uint8_t     *left;    /* for buf; for str, it should be NULL */
    };

    union {
        uint8_t *right;   /* for buf */
        int64_t  len;     /* for str */
    };

    int64_t  capacity;    /* for buf, it is total buf size.
                             For str with trailing \0 it is len+1 */
    int8_t   bytes_owned; /* if it is my responsibility to free bytes, when destroy */
    uint8_t *bytes;
    /* st_mempool_t */
};

struct st_MemPool;
static uint8_t *empty_str_ __attribute__((unused)) = (uint8_t *)"";

#define st_str_wrap_(_type, _len, _capacity, _owned, _bytes)                  \
        {                                                                     \
            .type        =(_type),                                            \
            .len         =(_len),                                             \
            .capacity    =(_capacity),                                        \
            .bytes_owned =(_owned),                                           \
            .bytes       =(uint8_t*)(_bytes)                                  \
        }

#define st_str_wrap_common(_charptr, _type, _len)     \
    st_str_wrap_((_type),                             \
                 (_len),                              \
                 (_len) + (_type == ST_TYPES_STRING), \
                 0,                                   \
                 (_charptr))

/* lcb: len, capacity and bytes */
#define st_str_wrap_lcb(_len, _capacity, _bytes) st_str_wrap_(ST_TYPES_STRING, (_len), (_capacity), 0, (_bytes))

#define st_str_wrap(_charptr, _len)   st_str_wrap_lcb((_len),         (_len),         (_charptr))
#define st_str_const(s)               st_str_wrap_lcb(sizeof(s)-1,    sizeof(s),      (s))
#define st_str_local(_len)            st_str_wrap_lcb((_len),         (_len),         (char[_len]){0})
#define st_str_wrap_0(_charptr, _len) st_str_wrap_lcb((_len),         (_len) + 1,     (_charptr))
#define st_str_wrap_chars(_chars)     st_str_wrap_lcb(sizeof(_chars), sizeof(_chars), (_chars))
#define st_str_zero                   st_str_wrap_lcb(0,              0,              NULL)
#define st_str_empty                  st_str_wrap_lcb(0,              1,              empty_str_)

/* deprecated: TODO remove _var and _null */
#define st_str_var  st_str_local
#define st_str_null st_str_zero

#define st_str_equal(a, b)                                                    \
        st_str_equal_((st_str_t*)(a), (st_str_t*)(b))

/*
 * use (uintptr_t)a == 0 instead of a == NULL
 * `a != NULL` raises warning about a is never NULL for const args
 */
#define st_str_equal_(a, b)                                                   \
  ((uintptr_t)(a) != 0                                                        \
   && (uintptr_t)(b) != 0                                                     \
   && (a)->bytes != NULL                                                      \
   && (b)->bytes != NULL                                                      \
   && (a)->len == (b)->len                                                    \
   && st_memcmp((a)->bytes, (b)->bytes, (a)->len) == 0)

#define st_str_n_arr_(...) \
    (sizeof((st_str_t*[]){__VA_ARGS__}) / sizeof(st_str_t*)), \
    (st_str_t*[]){__VA_ARGS__}

#define st_str_trailing_0(str) ((str)->len == (str)->capacity - 1)

/*
 * // Join string to a full path.
 * // Result rst stores one more byte for trailing '\0'.
 * // Thus it is safe to use rst->bytes directly in posix API.
 * #define st_str_join_path(s, ...)      st_str_join_with_n((s), &(st_str_t)st_str_const("/"), st_str_n_arr_(__VA_ARGS__))
 * #define st_str_join_with(s, sep, ...) st_str_join_with_n((s), (sep), st_str_n_arr_(__VA_ARGS__))
 *
 * int st_str_join_with_n(st_str_t *s, st_str_t *sep, int n, st_str_t **strs);
 */

int st_str_init(st_str_t *string, int64_t len);
int st_str_init_0(st_str_t *string, int64_t len);

void st_str_destroy(st_str_t *string);

void st_str_ref(st_str_t *s, const st_str_t *target);

int st_str_copy(st_str_t *str, const st_str_t *from);
int st_str_copy_cstr(st_str_t *str, const char *s);

/*
 * // NOTE: log depends on string. implement it manually.
 * // return : st__ERR_BUF_OVERFLOW when buffer not enough. no copy will be done under this situation
 * //    pos : length do not contain the end '\0'
 * int st_str_to_str(const st_str_t *ptr, char *buf, const int64_t len, int64_t *pos);
 */

/*
 * static inline int64_t st_str_len(const st_str_t *ptr) {
 *   return ptr->len;
 * }
 */

/*
 * static inline const char *st_str_to_cstring(const st_str_t *s) {
 *
 *   if (s == NULL) {
 *     return st__LITERAL_NULL;
 *   }
 *
 *   if (s->trailing_0) {
 *     return s->bytes;
 *   }
 *
 *   char *buf = st_utility_get_tostring_buf();
 *   int64_t pos = 0;
 *   int ret = st_str_to_str(s, buf, st__TO_CSTRING_BUFFER_SIZE, &pos);
 *   if (ret != st__OK) {
 *     buf[0] = '\0';
 *   }
 *   return buf;
 * }
 */

/* take ownership of bytes from another st_str_t 'from'. */
int st_str_seize(st_str_t *str, st_str_t *from);


// thread_safe: no
int st_str_cmp(const st_str_t *a, const st_str_t *b);

struct st_Buf;

int st_str_increment(st_str_t *str);

/*
 * //note st_str_size is use for serialize buf size
 * int64_t st_str_size(void *val);
 * int st_str_serialize(void *val, struct st_Buf *buf);
 * int st_str_deserialize(void *val, struct st_Buf *buf);
 * //cast functions
 * int st_str_to_int(const struct st_str_t* str, int64_t* int_val);
 * int st_str_to_double(const struct st_str_t* str, double* double_val);
 * int st_str_to_float(const struct st_str_t* str, float* float_val);
 * int st_str_to_bool(const struct st_str_t* str, int32_t* bool_val);
 * int st_str_to_time(const struct st_str_t* str, time_t* time_val);
 */

#ifdef __cplusplus
}
#endif
#endif /* ST_STR_ */
// vim:ts=4:sw=4:et
