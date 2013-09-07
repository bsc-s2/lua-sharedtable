#ifndef __INC__STR_H__
#define __INC__STR_H__

#include <stdint.h>

#define ST_STR_MAXLEN ( (1<<15) - 1 )

typedef struct st_str_s st_str_t;

struct st_str_s {
    int16_t  len;
    uint8_t  owned; /* if I am in charge of freeing bytes */
    uint8_t *bytes;
};

#define st_str_const(s) { .len=sizeof(s) - 1, .owned=0, .bytes=s }

#endif /* __INC__STR_H__ */
