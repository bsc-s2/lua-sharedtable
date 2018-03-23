#ifndef __ST_INC_TYPES_INCLUDE__
#define __ST_INC_TYPES_INCLUDE__

typedef unsigned char st_bool;

/** keep types value compatible with luajit-2.1 */
typedef enum st_types_e {
    ST_TYPES_UNKNOWN    = -1,

    ST_TYPES_NIL        = 0x00,
    ST_TYPES_BOOLEAN    = 0x01,
    ST_TYPES_NUMBER     = 0x03,
    ST_TYPES_INTEGER    = 0x13,
    ST_TYPES_U64        = 0x23,
    ST_TYPES_STRING     = 0x04,
    ST_TYPES_CHAR_ARRAY = 0x14,
    ST_TYPES_TABLE      = 0x17,

} st_types_t;

static inline int
st_types_is_table(st_types_t type)
{
    return (type == ST_TYPES_TABLE);
}

#endif
