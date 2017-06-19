TOPDIR=.
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
EXENAME=
OUTPUT_DIR=

include Makefile.env

all:
	make -C src

.PHONY: test
test: all
	make -C test

.PHONY: clean
clean: 
	make -C src clean
	make -C test clean


