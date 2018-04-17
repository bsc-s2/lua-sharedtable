
src       = slab.c
target    = slab.a
libs      = pthread rt
deps      = \
	    bitmap \
	    pagepool \
	    array \
	    rbtree \
	    region \
	    robustlock \
	    util \

test_exec = test_slab

BASE_DIR ?= $(CURDIR)/..
include $(BASE_DIR)/def.mk
