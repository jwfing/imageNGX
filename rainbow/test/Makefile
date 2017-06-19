VERSION_ID=test_imagickRainbow 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
TESTEXENAME=test_imagickRainbow
OUTPUT_DIR=$(TOPDIR)/output
CPPUNIT_DIR=/usr/lib/cppunit

include Makefile.env

CFLAGS+= -I$(CPPUNIT_DIR)/include -I$(INCDIR) -I/usr/local/include -DDEBUG -pg -fprofile-arcs -ftest-coverage -I/usr/local/include/ImageMagick

CXXFLAGS=$(CFLAGS)

LDFLAGS= -L$(OUTPUT_DIR) -L/usr/local/lib
LDLIBS= -llog4cxx -limagickrainbow -lMagick++ -lMagickWand -lMagickCore -lcppunit -fprofile-arcs -ftest-coverage

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

TARGETS = test_imagickRainbow

.PHONY: all
all:$(TARGETS)
	for CMD in $(TARGETS); do (./$$CMD); done

test_edgedetect: test_imagickrainbow.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean: clean_test

clean_test:
	$(RM) $(TARGETS)
	$(RM) *.o
	$(RM) $(CLEANFILES)
