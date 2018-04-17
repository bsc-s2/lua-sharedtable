
src       = gc.c
target    = gc.a
libs      = pthread rt
deps      = \
	    table      \
	    pagepool   \
	    rbtree     \
	    region     \
	    robustlock \
	    slab       \
	    bitmap     \
	    array      \
	    str        \
	    util       \

test_exec = test_gc

BASE_DIR ?= $(CURDIR)/..
include $(BASE_DIR)/def.mk
