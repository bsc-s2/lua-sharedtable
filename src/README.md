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

Makefile

# Status

This library is considered production ready.

# Synopsis
Compile project and do test at the same time.

```
make
```

Compile project and do test at the same time.

```
make test
```

Compile project and do benchmark test.

```
make bench
```

Clean up

```
make clean
```

For all the commands above, you can add the following compile options:

- `build`: build level
    - `release`: release version which is default.
    - `test`: test version.
- `debug`: debug mode
    - `0`: turn off debug mode which is default.
    - `1`: turn on debug mode.
- `debug_unittest `: debug unittest
    - `0`: turn off debug mode which is default.
    - `1`: turn on debug mode.
- `mem`: memory size level
    - `small`: use small memory size.
    - `normal`: use normal memory size which is default.

# Description
Makefile for GNU make tool.

The Makefile on the top level is used to call Makefiles of submodules to do compiling and test.

The core of the whole compiling system is def.mk. It defines `target`, `prerequisites` and `commands`.
At the meanwhile, it exposes multiple variables to submodules to make them capable of choosing their own way to be compiled.
It also handles module dependences by the instruction of module Makefile variable `deps`.

# DataTypes
```
src       = region.c
target    = region.a
test_exec = test_region
deps      = robustlock
libs      = rt pthread
cflags    = -g

BASE_DIR ?= $(CURDIR)/..
include $(BASE_DIR)/def.mk
```

- `src`: source file
- `target`: target static library file to make
- `test_exec`: module test program
- `deps`: dependent submodules list
- `libs`: dependent libraries list
- `cflags`: module additional compile flags


# Author
Zhang Yanpo (张炎泼) <xp@baishancloud.com>

# Copyright and License

The MIT License (MIT)

Copyright (c) 2017 Zhang Yanpo (张炎泼) <xp@baishancloud.com>
