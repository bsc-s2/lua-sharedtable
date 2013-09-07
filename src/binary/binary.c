#include "binary.h"

int64_t
bin_vu64_size(uint64_t x) {

    int64_t l = 0;

    do {
        l++;
        x >>= 7;
    } while (x != 0);

    return l;
}

int64_t
bin_vu64_put(uint8_t *buf, uint64_t len, uint64_t x) {

    int64_t i = 0;

    if (x == 0) {
        if (len > 0) {
            buf[0] = 0;
            return 1;
        } else {
            return ST_BUF_OVERFLOW;
        }
    }

    for (i = 0; x != 0 && i < len; i++) {
        buf[i] = ((uint8_t)x) | 0x80;
        x >>= 7;
    }

    if (x > 0) {
        return ST_BUF_OVERFLOW;
    }

    buf[i - 1] &= 0x7f;

    return i;
}

int64_t
bin_vu64_load(uint8_t *buf, uint64_t len, uint64_t *rst) {

    int64_t  i;
    uint64_t x;
    int      shift;
    uint8_t  b;

    x = 0;
    shift = 0;

    if (rst == NULL) {
        return ST_ARG_INVALID;
    }

    for (i = 0; i < len; i++) {
        b = buf[i];

        if (b < 0x80) {
            if (i > 9 || (i == 9 && b > 1)) {
                return ST_NUM_OVERFLOW;
            }
            x |= ((uint64_t)b) << shift;
            break;
        } else {
            x |= ((uint64_t)(b & 0x7f)) << shift;
            shift += 7;
        }
    }

    if (i == len) {
        return ST_BUF_NOT_ENOUGH;
    }

    *rst = x;

    return i + 1;
}
