VERSION_ID = test 1.0.0.0 64bit

VERSION_SIGN=-D__64BIT__

EX_CHECK=-Wfloat-equal

CC=g++
CXX=g++
LD=ld
AR=ar cr
CP=cp -f
RM=rm -f
MKDIR=mkdir -p
RMDIR=rm -Rf

CPPUNIT_DIR=/usr/lib/cppunit
TOPDIR=..
INCDIR=$(TOPDIR)/include
THUMBNAILER_DIR=$(TOPDIR)/../thumbnail

INCLUDE = -I$(CPPUNIT_DIR)/include -I$(INCDIR) -I/usr/local/include -I/usr/local/include/ImageMagick \
	 -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -I$(THUMBNAILER_DIR)/include
LDFLAGS = -L$(THUMBNAILER_DIR)/output -L$(TOPDIR)/output -L/usr/local/lib -L/usr/lib
LDLIBS = -llog4cxx -lboost_system -lboost_thread-mt -lboost_filesystem -lthumbnailer -lMagick++ -lMagickCore \
		 -limageservice -lmongoc-1.0 -lpion -lcppunit -fprofile-arcs -ftest-coverage 
CFLAGS = -g -Wall -Wextra -Wno-unused-parameter -Wformat -Wconversion \
	 -Wdeprecated -D__FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -fprofile-arcs -ftest-coverage \
	 -D_REENTRANT -frtti $(EX_CHECK) -finline-functions $(INCLUDE) \
	 -D__VERSION_ID__="\"$(VERSION_ID)\"" $(VERSION_SIGN)
CXXFLAGS = $(CFLAGS) -DDEBUG -pg

CLEANFILES = *.o
COVERAGEFILES = *.gcno *.gcda *.gcov 

TARGETS = test_config test_gridfs test_localstorage test_mempool test_largemem
#test_cropService test_s3
#TARGETS = test_s3

all:$(TARGETS)
#	./test_s3
	for CMD in $(TARGETS); do (./$$CMD); done

test_config: test_config.o

test_cropService: test_cropService.o

test_gridfs: test_gridfs.o

test_localstorage: test_localstorage.o

test_mempool: test_mempool.o
test_largemem: test_largemem.o
test_s3: test_s3.o

.PHONY: clean
clean:
	$(RM) $(TARGETS)
	$(RM) $(CLEANFILES)
	$(RM) $(COVERAGEFILES)

