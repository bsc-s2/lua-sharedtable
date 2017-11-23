# Compilation time switcher

## Usage:

```
make FOO=bar
```

## Define a switcher

To define a switcher, define several **value** macros in a `.c` or `.h` file where you
want to use them.
Each of them should has the same prefix, in our case, it is `st_foomode_`.

If `make` is run with macro definition `make FOO=small`, the progrom then can
use `st_switcher_get(st_foomode_)` to retreive the value of macro
`st_foomode_small`.

Then the following program can use `st_foomode`.


```c
// make FOO=small

// define valid values of st_foomode_<FOO>
#define st_foomode_small 2
#define st_foomode_normal 5
#define st_foomode_extra 100

// retreive command line macro FOO=small(`st_foomode_<FOO>`)
// and save it in `st_foomode`.
#define st_foomode st_switcher_get(st_foomode_, FOO)

// use it
#if st_foomode == st_foomode_small
    // yes
#else
   // no
#endif
```
