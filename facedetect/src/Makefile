VERSION_ID=facedetect 1.0.0.0 64bit

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
LIBNAME=libfacedetect.a
OUTPUT_DIR=$(TOPDIR)/output

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

CXXFLAGS=$(CFLAGS)

LDFLAGS= -L$(LIBDIR) -L/usr/local/lib
LDLIBS= -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_core

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

OBJS=faceDetector.o

all:$(OUTPUT_DIR)/$(LIBNAME)

$(OUTPUT_DIR)/$(LIBNAME): $(OBJS)
	if [ ! -d $(OUTPUT_DIR) ]; then $(MKDIR) $(OUTPUT_DIR); fi
	$(RM) $@
	$(AR) $@ $^

.PHONY: clean
clean: 
	$(RM) $(OUTPUT_DIR)/$(LIBNAME)
	$(RM) $(CLEANFILES)
