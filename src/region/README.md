<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
#   Table of Content

- [Name](#name)
- [Status](#status)
- [Synopsis](#synopsis)
- [Description](#description)
- [DataTypes](#structure)
- [Author](#author)
- [Copyright and License](#copyright-and-license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Name

region

# Status

This library is considered production ready.

# Synopsis

Usage of creating and destroying OSIX-based shared memory space.

```
#include <stdlib.h>

#include "region.h"


#define CONFIG_REGION               10
#define PAGES_PER_REGION            1024
#define REGION_NUM                  10
#define PAGE_SIZE                   sysconf(_SC_PAGESIZE)


int main()
{
    int shm_fd        = 0;
    uint8_t *addr     = NULL;
    ssize_t reg_size  = PAGES_PER_REGION * PAGE_SIZE;
    ssize_t data_len  = reg_size * REGION_NUM;
    ssize_t cfg_len   = CONFIG_REGION * PAGE_SIZE;
    ssize_t length    = data_len + cfg_len;

    /** create shm area */
    int ret = st_region_shm_create(length, (void **)&addr, &shm_fd);
    if (ret != ST_OK) {
        /** your error handling code */
        exit(1);
    }

    st_region_t *rcb       = (st_region_t *)addr;
    uint8_t *data_base_ptr = (uint8_t *)((uintptr_t)addr + cfg_len);

    /** initialize shm region control block */
    ret = st_region_init(rcb,
                            data_base_ptr,
                            PAGES_PER_REGION,
                            REGION_NUM,
                            0);
    if (ret != ST_OK) {
        /** your error handling code */
        exit(2);
    }

    /** alloc regions */
    uint8_t *ret_addr = NULL;
    uint8_t *reg_addr = NULL;
    for (int idx = 0; idx < REGION_NUM; idx++) {
        /** get region addr */
        reg_addr = st_region_base_addr_by_idx(rcb, idx);

        ret = st_region_alloc_reg(rcb, &ret_addr);
        if (ret != ST_OK) {
            /** your error handling code */
            exit(3);
        }
    }

    /** free region by index */
    int idx  = REGION_NUM / 2;
    reg_addr = st_region_base_addr_by_idx(rcb, idx);

    ret = st_region_free_reg(rcb, reg_addr);
    if (ret != ST_OK) {
        /** your error handling code */
        exit(4);
    }

    ret = st_region_destroy(rcb);
    if (ret != ST_OK) {
        /** your error handling code */
        exit(5);
    }

    ret = st_region_shm_destroy(shm_fd, addr, length);
    if (ret != ST_OK) {
        /** your error handling code */
        exit(6);
    }
}

```

# Description
a region is a block of continuous memory space which has its own base address,
size and state.

region module creates and destroys POSIX-based shared memory area. It logically
devides this continuous memory area into multiple regions and stores all the
information. region module supplies interfaces to allocate and free regions.

caller can choose to use region module internal lock to protect struct or not
in initialization.

# DataTypes

```
typedef enum st_region_state_e {
    ST_REGION_STATE_FREE = 0x01,
    ST_REGION_STATE_BUSY = 0x02,
} st_region_state_t;


typedef struct st_region_s st_region_t;

struct st_region_s {
    uint8_t           *base_addr;
    ssize_t           page_size;
    int64_t           reg_cnt;
    ssize_t           reg_size;
    st_region_state_t states[ST_REGION_MAX_NUM];
    pthread_mutex_t   lock;
    int               use_lock;
};
```
- `st_region_state_t`: region is free or busy
- `base_addr`: the base address of the whole memory area
- `page_size`: page size used by region module
- `reg_cnt`: the number of regions
- `reg_size`: the size of each region in bytes
- `states`: state of each regions
- `lock`: pthread mutex lock to protect struct
- `use_lock`: use lock or not

# Author

李树龙 <shulong.li@baishancloud.com>

# Copyright and License

The MIT License (MIT)

Copyright (c) 2017 李树龙 <shulong.li@baishancloud.com>
