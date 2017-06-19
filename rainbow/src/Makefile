VERSION_ID=rainbow 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
LIBNAME=libimagickrainbow.a
OUTPUT_DIR=$(TOPDIR)/output
THUMBNAIL_DIR = $(TOPDIR)/../thumbnail
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

CXXFLAGS=$(CFLAGS) -I/usr/local/include/ImageMagick -I$(THUMBNAIL_DIR)/include -I$(WEBSERVICE_DIR)/include

LDFLAGS= -L$(LIBDIR) -L/usr/local/lib
LDLIBS= -lMagick++ -lMagickCore -lMagickWand

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

OBJS=imagickRainbow.o

all:$(OUTPUT_DIR)/$(LIBNAME)

$(OUTPUT_DIR)/$(LIBNAME): $(OBJS)
	if [ ! -d $(OUTPUT_DIR) ]; then $(MKDIR) $(OUTPUT_DIR); fi
	$(RM) $@
	$(AR) $@ $^

.PHONY: clean
clean: 
	$(RM) $(OUTPUT_DIR)/$(LIBNAME)
	$(RM) $(CLEANFILES)

