#ifndef __BINARY__BINARY_H__
#     define __BINARY__BINARY_H__


#include <stdint.h>
#include <stdio.h>

#include "inc/err.h"

int64_t bin_vu64_size(uint64_t x);
int64_t bin_vu64_put(uint8_t *buf, uint64_t len, uint64_t x);
int64_t bin_vu64_load(uint8_t *buf, uint64_t len, uint64_t *rst);

#endif /* __BINARY__BINARY_H__ */
