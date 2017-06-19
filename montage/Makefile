TOPDIR=.
INCDIR=$(TOPDIR)/include
OUTPUT_DIR=

include Makefile.env

TEST_FILES=test_edgedetect.cpp
COVFILE=$(shell /bin/pwd)/test.cov
export COVFILE

all:
	make -C src

.PHONY: test
test: all
	make -C test

test-cover:
	make -C src INDEBUGGING=2
	make -C test
	mkdir -p cover
	cp include/*.h src/*.cpp src/*.gcda src/*.gcno test/*.h test/*.cpp test/*.gcda test/*.gcno cover/
	cd cover && gcov $(TEST_FILES) && lcov -d . -t 'cover test' -o 'cover.info' -b . -c
	cd cover && genhtml -o cover_result cover.info

.PHONY: clean
clean: 
	make -C src clean
	make -C test clean

