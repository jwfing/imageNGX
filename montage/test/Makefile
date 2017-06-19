VERSION_ID=test_imagickmontage 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
TESTEXENAME=test_imagickMontage
OUTPUT_DIR=$(TOPDIR)/output
CPPUNIT_DIR=/usr/lib/cppunit

include Makefile.env

CFLAGS+= -I$(CPPUNIT_DIR)/include -I$(INCDIR) -I/usr/local/include -DDEBUG -pg -fprofile-arcs -ftest-coverage -I/usr/local/include/ImageMagick

CXXFLAGS=$(CFLAGS)

LDFLAGS= -L$(LIBDIR) -L$(OUTPUT_DIR) -L/usr/local/lib
LDLIBS= -llog4cxx -limagickmontage -lMagick++ -lMagickWand -lMagickCore -lcppunit -fprofile-arcs -ftest-coverage

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

TARGETS = test_imagickMontage

.PHONY: all
all:$(TARGETS)
	for CMD in $(TARGETS); do (./$$CMD); done

test_edgedetect: test_imagickmontage.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean: clean_test

clean_test:
	$(RM) $(TARGETS)
	$(RM) *.o
	$(RM) $(CLEANFILES)
