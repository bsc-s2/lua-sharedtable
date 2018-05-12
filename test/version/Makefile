# this is demo to show how to install and use shared table.
#
# because having to avoid name conflicttion,
# all of the target names are weird

src       = test_lib.c
target    = test_lib.a
deps      = version

target_dylib = libst_test_lib.so

version: dylib install self run

install:
	# soname is used by loader
	# use ldconfig to create this link in install script,
	ln -sf $(target_dylib).$(st_version_full) $(target_dylib).$(st_version_soname)
	# this name is used by linker
	ln -sf $(target_dylib).$(st_version_soname) $(target_dylib)

self:
	$(CC) -o test test_exe.c -L./ -lst_test_lib

run:
	/lib64/ld-2.17.so --library-path ./ ./test

tidy:
	rm -f $(target_dylib)*
	rm test
	rm *.o
	rm *.a

BASE_DIR ?= $(CURDIR)/../../src
include $(BASE_DIR)/def.mk
