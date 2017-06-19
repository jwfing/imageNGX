VERSION_ID=test_facedetect 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
TESTEXENAME=test_thumbnailer
OUTPUT_DIR=$(TOPDIR)/output
FACE_DETECT_DIR=$(TOPDIR)/../facedetect

include Makefile.env

CFLAGS+= -I$(INCDIR) -DDEBUG -pg
CXXFLAGS=$(CFLAGS) -I/usr/local/include/ImageMagick -I$(FACE_DETECT_DIR)/include

LDFLAGS= -L$(LIBDIR) -L$(OUTPUT_DIR) -L/usr/local/lib -L$(FACE_DETECT_DIR)/output

USING_FACEDETECT=1

ifeq ($(USING_FACEDETECT), 1)
	LDLIBS= -lthumbnailer -lcppunit -lMagick++ -lMagickCore -lMagickWand -lfacedetect -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_core -llog4cxx
else
	LDLIBS= -lthumbnailer -lcppunit -lMagick++ -lMagickCore -lMagickWand -llog4cxx
endif

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

TARGETS = test_thumbnailer test_imagicImageOperator

.PHONY: all
all:$(TARGETS)
	for CMD in $(TARGETS); do (./$$CMD); done

test_thumbnailer: test_imagickThumbnailer.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

test_imagicImageOperator: test_imagicImageOperator.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGETS)
	$(RM) $(CLEANFILES)
