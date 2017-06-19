VERSION_ID=test_facedetect 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
TESTEXENAME=test_facedetect
OUTPUT_DIR=$(TOPDIR)/output
CPPUNIT_DIR=/usr/lib/cppunit

include Makefile.env

CFLAGS+= -I$(CPPUNIT_DIR)/include -I$(INCDIR) -I/usr/local/include -DDEBUG -pg -fprofile-arcs -ftest-coverage

CXXFLAGS=$(CFLAGS)

LDFLAGS= -L$(OUTPUT_DIR) -L/usr/local/lib
LDLIBS= -llog4cxx -lfacedetect -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect -lopencv_core -lcppunit -fprofile-arcs -ftest-coverage

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

TARGETS = test_facedetect

.PHONY: all
all:$(TARGETS)
	for CMD in $(TARGETS); do (./$$CMD); done

test_facedetect: test_facedetect.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean: clean_test

clean_test:
	$(RM) $(TARGETS)
	$(RM) *.o
	$(RM) $(CLEANFILES)
