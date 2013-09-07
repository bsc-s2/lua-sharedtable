#ifndef _CHKSUM_CRC32_H_INCLUDED_
#define _CHKSUM_CRC32_H_INCLUDED_

#include <stdint.h>

extern uint32_t   crc32_table256[];


static inline uint32_t
crc32(unsigned char *p, size_t len)
{
    uint32_t  crc;

    crc = 0xffffffff;

    while (len--) {
        crc = crc32_table256[(crc ^ *p++) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xffffffff;
}

static inline void
crc32_update(uint32_t *crc, unsigned char *p, size_t len)
{
    uint32_t  c;

    c = *crc;

    while (len--) {
        c = crc32_table256[(c ^ *p++) & 0xff] ^ (c >> 8);
    }

    *crc = c;
}


#endif /* _CHKSUM_CRC32_H_INCLUDED_ */
