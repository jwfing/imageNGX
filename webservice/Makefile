TOPDIR=.
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
EXENAME=
OUTPUT_DIR=

CP=cp -f -r
RM=rm -f
MKDIR=mkdir -p
RMDIR=rm -Rf

TEST_FILES = test_config.cpp test_gridfs.cpp test_mempool.cpp test_cropService.cpp
COVFILE=$(shell /bin/pwd)/test.cov

export COVFILE

all:
	make -C src
	$(CP) conf output/

.PHONY: test
test: all
	make -C test

test-cover:
	make -C src INDEBUGGING=2
	make -C test
	$(MKDIR) cover
	$(CP) include/*.h src/*.cpp src/*.gcda src/*.gcno test/*.h test/*.cpp test/*.gcda test/*.gcno cover/
	cd cover && gcov $(TEST_FILES) && lcov -d . -t 'cover test' -o 'cover.info' -b . -c
	cd cover && genhtml -o cover_result cover.info

.PHONY: clean
clean:
	make -C src clean
	make -C test clean

