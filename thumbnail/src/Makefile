TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
LIBNAME=libthumbnailer.a
OUTPUT_DIR=$(TOPDIR)/output
FACE_DETECT_DIR=$(TOPDIR)/../facedetect
WEBSERVICE_DIR=$(TOPDIR)/../webservice

include Makefile.env

CFLAGS += -I$(INCDIR)

ifeq ($(INDEBUGGING), 2)
	CFLAGS += -DDEBUG -pg -fprofile-arcs -ftest-coverage
else
ifeq ($(INDEBUGGING), 1)
	CFLAGS += -DDEBUG -pg -DPERF_MONITOR
else
	CFLAGS += -DNDEBUG -O2
endif
endif

CXXFLAGS=$(CFLAGS) -I/usr/local/include/ImageMagick -I$(FACE_DETECT_DIR)/include -I$(WEBSERVICE_DIR)/include

LDFLAGS= -L$(LIBDIR) -L/usr/local/lib -L$(FACE_DETECT_DIR)/output

USING_FACEDETECT=1
ifeq ($(USING_FACEDETECT), 1)
	LDLIBS= -lMagick++ -lMagickCore -lMagickWand -lfacedetect
else
	LDLIBS= -lMagick++ -lMagickCore -lMagickWand
endif

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

OBJS=imagickThumbnailer.o thumbnailer.o thumbnailerFactory.o imagicImageOperator.o imagickExtraImageInfo.o


all: $(OUTPUT_DIR)/$(LIBNAME)

$(OUTPUT_DIR)/$(LIBNAME): $(OBJS)
	if [ ! -d $(OUTPUT_DIR) ]; then $(MKDIR) $(OUTPUT_DIR); fi
	$(RM) $@
	$(AR) $@ $^

.PHONY: test
test: all

.PHONY: clean
clean: clean_output clean_test

clean_output:
	$(RM) $(OUTPUT_DIR)/$(LIBNAME)

clean_test:
	$(RM) $(CLEANFILES)

