#   Name

unittest

#   Status

This library is in beta phase.

#   Description

It provides with a simple unittest and benchmarking lib.

#   Synopsis


```c

#include "unittest/unittest.h"

/* define test case */

st_test(module_name, case_name) {

    int i = 1;
    st_ut_eq(1, i, "i=%lu", i);

}

/* define benchmark that runs for at least 2 seconds */

void *init_f() {
    static char *b = "b";
    return (void*)b;
}
void clean_f(void *data) {
    return;
}

st_bench(module_name, bench_name, init_f, clean_f, 2, data, n) {
    int i;
    for (i = 0; i < n; i++) {
        memcmp("a", (char*)data, 1);
    }
}

/* define a simplified benchmark without init_f/clean_f */

st_ben(module_name, bench_name, 2, n) {
    int i;
    for (i = 0; i < n; i++) {
        memcmp("a", "b", 1);
    }
}

/* start test and benchmarks */
st_ut_main;
```

#   Methods

##  st_test

**syntax**:
`st_test(module_name, case_name) {...}`

It defines a unit test case by creating a function with body in the braces.
And function name is defined with `module_name` and `case_name`.

This function is called by macro `st_ut_main`;

These two literal string arguments have no programming meaning and is for naming
in test results, e.g.: `module_name::case_name`.


##  st_bench

**syntax**:
`st_bench(module_name, benchmark_name, init_func, clean_func, seconds_to_run, data, times){...}`

Similar to `st_test`, it defines a benchmark function with `module_name` and
`benchmark_name`.


**arguments**:

-   `init_func`:
    specifies a function to create data used by this benchmark.
    Its prototype is defined as `void *(*st_ut_bench_init_f)();`

    It could be `NULL`.

-   `clean_func`:
    specifies a function to clean what `init_func` created.
    Its prototype is defined as `void (*st_ut_bench_clean_f)(void *data);`

    It could be `NULL`.

-   `seconds_to_run`:
    specifies how many seconds to run a benchmark.

    This unit test framework invokes a benchmark several times with incremental
    `times` till the total running time exceeds `seconds_to_run`.

-   `data`:
    specifies the variable name to receive the data `init_func` returned.

    Inside the body(in braces), `data` is a `void*` pointer.

-   `times`:
    specifies the variable name to receive number of times to run.

    Inside the body(in braces), `times` is a `int64_t` integer.


##  st_ben

**syntax**:
`st_ben(module_name, benchmark_name, seconds_to_run, times){...}`

`st_ben` is a simplified version of `st_bench`.
It does not accept init/clean functions or receives a `data` argument.



#   Author

Zhang Yanpo (张炎泼) <drdr.xp@gmail.com>

#   Copyright and License

The MIT License (MIT)

Copyright (c) 2015 Zhang Yanpo (张炎泼) <drdr.xp@gmail.com>
