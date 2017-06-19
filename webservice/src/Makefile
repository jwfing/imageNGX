VERSION_ID=imageService 1.0.0.0 64bit
COMPATIBLE=0

TOPDIR=..
INCDIR=$(TOPDIR)/include
LIBDIR=$(TOPDIR)/../lib
EXENAME=imageService
STATICLIBNAME=libimageservice.a
OUTPUT_DIR=$(TOPDIR)/output
THUMBNAIL_DIR = $(TOPDIR)/../thumbnail
FACEDETECT_DIR = $(TOPDIR)/../facedetect
MONTAGE_DIR = $(TOPDIR)/../montage
RAINBOW_DIR = $(TOPDIR)/../rainbow

CP=cp -f
RM=rm -f
MKDIR=mkdir -p
RMDIR=rm -Rf
CC=gcc
CXX=g++
LD=ld
AR=ar cr

COVFILE=$(shell /bin/pwd)/test.cov

export COVFILE

CFLAGS= -g -Wall -Wformat -Wdeprecated -Wfloat-equal -Werror -Wno-unused-private-field \
	-Wextra -Wno-unused-parameter -finline-functions -I$(FACEDETECT_DIR)/include \
	-I$(INCDIR) -I/usr/local/include -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 \
        -I/usr/include -I/usr/local/include/ImageMagick -I$(THUMBNAIL_DIR)/include \
	-I$(MONTAGE_DIR)/include -I$(RAINBOW_DIR)/include -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D__VERSION_ID__="\"$(VERSION_ID)\"" \
	-D__COMPATIBLE__="\"$(COMPATIBLE)\"" -DBOOST_FILESYSTEM2_NARROW_ONLY 

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

LDFLAGS= -L$(LIBDIR) -L/usr/local/lib -L/usr/lib -L$(THUMBNAIL_DIR)/output -L$(FACEDETECT_DIR)/output -L$(MONTAGE_DIR)/output -L$(RAINBOW_DIR)/output

USING_FACEDETECT=1
ifeq ($(USING_FACEDETECT), 1)
	LDLIBS= -llog4cxx -lpthread -lcrypto -lssl -lboost_system -lboost_thread-mt -lboost_filesystem -lboost_program_options -lthumbnailer -lMagick++ -lMagickCore \
		-lmongoc-1.0 -lpion -limagickmontage \
		-lfacedetect -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect -lopencv_core -limagickrainbow
else
	LDLIBS= -llog4cxx -lboost_system -lboost_thread -lboost_filesystem -lthumbnailer -lMagick++ -lMagickCore \
		-limagickmontage -lmongoc-1.0 -lpion -limagickrainbow
endif

ifeq ($(INDEBUGGING), 2)
	LDLIBS += -fprofile-arcs -ftest-coverage
endif

CLEANFILES= *.o *.gc* core.* *.gdb *.out nulllog nulllog.wf

OBJS=config.o commonService.o defaultFavicon.o localStorage.o gridfsStorage.o thread.o cropService.o cropServiceV2.o statusService.o memoryPool.o storagePool.o stopWatch.o montageService.o rainbowService.o fileService.o

.PHONY: all
all: $(OUTPUT_DIR)/$(EXENAME) $(OUTPUT_DIR)/$(STATICLIBNAME)

$(OUTPUT_DIR)/$(EXENAME): $(OBJS) main.o
	if [ ! -d $(OUTPUT_DIR) ]; then $(MKDIR) $(OUTPUT_DIR); fi
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OUTPUT_DIR)/$(STATICLIBNAME): $(OBJS)
	if [ ! -d $(OUTPUT_DIR) ]; then $(MKDIR) $(OUTPUT_DIR); fi
	$(RM) $@
	$(AR) $@ $^

.PHONY: clean
clean: clean_output
	$(RM) $(CLEANFILES)

clean_output:
	$(RM) $(OUTPUT_DIR)/$(EXENAME)
	$(RM) $(OUTPUT_DIR)/$(STATICLIBNAME)

