#ifndef __ST_INC_TYPES_INCLUDE__
#define __ST_INC_TYPES_INCLUDE__

typedef enum st_types_e {
    ST_TYPES_UNKNOWN = -1,

    ST_TYPES_STRING  = 0x00,
    ST_TYPES_BOOLEAN = 0x01,
    ST_TYPES_NUMBER  = 0x02,
    ST_TYPES_TABLE   = 0x03,

} st_types_t;

static inline int
st_types_is_table(st_types_t type)
{
    return (type == ST_TYPES_TABLE);
}

#endif
