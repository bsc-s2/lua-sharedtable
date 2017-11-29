<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
#   Table of Content

- [Name](#name)
- [Status](#status)
- [Synopsis](#synopsis)
- [Description](#description)
- [Structure](#structure)
- [Author](#author)
- [Copyright and License](#copyright-and-license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Name

array

Support static and dynamic array, you can insert, remove, sort, search array.

# Status

This library is considered production ready.

# Synopsis

Usage with static array

```

#include "array.h"

int main()
{
    st_array_t array = {0};
    int array_buf[10];

    int ret = st_array_init_static(&array, sizeof(int), array_buf, 10, NULL);
    if (ret != ST_OK) {
        return ret;
    }

    int append_v = 2;

    ret = st_array_append(&array, &append_v);
    if (ret != ST_OK) {
        return ret;
    }

    int insert_v = 12;

    ret = st_array_insert(&array, 1, &insert_v);
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_array_remove(&array, 0);
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_array_destroy(&array);
    if (ret != ST_OK) {
        return ret;
    }

    return ret;
}

```

Usage with dynamic array

```

#include "array.h"

int compare(const void *a, const void *b)
{
    int m = *(int *) a;
    int n = *(int *) b;

    return m-n;
}

int main()
{
    st_array_t array = {0};
    int append_buf[10] = {9, 8, 0, 1, 2, 3, 4, 5, 6, 7};

    st_callback_memory_t callback = {.pool = NULL, .realloc = _realloc, .free = _free,}
                 st_array_init_dynamic(c.array, 4, c.callback, compare),

    int ret = st_array_init_dynamic(&array, sizeof(int), callback, compare);
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_array_append(&array, append_buf, 10);
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_array_sort(&array, NULL);
    if (ret != ST_OK) {
        return ret;
    }

    int value = 6;
    ssize_t idx;

    ret = st_array_bsearch_left(&array, &value, NULL, &idx),
    if (ret != ST_OK) {
        return ret;
    }

    ret = st_array_bsearch_right(&array, &value, NULL, &idx),
    if (ret != ST_OK) {
        return ret;
    }

    return ret;
}
```

# Description

Support static and dynamic array, can insert, remove, sort, search etc operation

If you already has array space, and don't need to dynamic extend,
you can use st_array_init_static to init array, and the total elements count is fixed.
then you can use array functions to handle it, example: insert, sort etc.

If you don't have array space, want to dynamic allocate array space,
you can use st_array_init_dynamic to init array,
and the total elements count can be increased when needed.
then you can use array functions to handle it, example: insert, sort etc.

# Structure

```
struct st_array_s {
    void *start_addr;

    ssize_t element_size;
    ssize_t current_cnt;
    ssize_t total_cnt;

    int dynamic;

    st_callback_memory_t callback;
    st_array_compare_f compare;
    int inited;
};
```

- start_addr: array space start address, all the elements will store in this space.

- element_size: element to store in array size. array is no element type,
    so you need to tell array how large the element is.

- current_cnt: currently the array has elements count.

- total_cnt: array can support total elements count. the value is fixed in static array,
    and can be increased in dynamic array.

- dynamic: identify the array is static or dynamic.

- callback: it is used for dynamic array, to increase array space or free space.

- compare: it is used to compare array element, it can be used for sort, search.
    if you do not need to search or sort array, it can be NULL.

# Author

cc (陈闯) <chuang.chen@baishancloud.com>

# Copyright and License

The MIT License (MIT)

Copyright (c) 2017 cc (陈闯) <chuang.chen@baishancloud.com>
