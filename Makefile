#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@sastrugi.colorado.edu
#========================================================================
SHELL = /bin/sh
TOP = /usr/local
LOCAL_LIB = $(TOP)/lib
LOCAL_INCLUDE = $(TOP)/include
CC = /usr/bin/cc
AR = /usr/bin/ar
RANLIB = /bin/touch
CO = /usr/sbin/co
INSTALL = /bin/cp
RM = /bin/rm -f
TAR = /bin/tar
COMPRESS = /usr/bsd/compress
TARFILE = maps.tar
DEBUG_FLAGS = -O
CLIBS = -lm
CFLAGS = -I$(LOCAL_INCLUDE) $(DEBUG_FLAGS)
SOURCES = mapx.c grids.c cdb.c maps.c
HEADERS = mapx.h grids.h cdb.h maps.h cdb_byteswap.h
OBJECTS = mapx.o grids.o cdb.o maps.o

all : libmaps.a install

libmaps.a : $(HEADERS) $(OBJECTS)
	$(AR) ruv libmaps.a $(OBJECTS)
	$(RANLIB) libmaps.a

install : libmaps.a $(HEADERS)
	$(INSTALL) libmaps.a $(LOCAL_LIB)
	$(INSTALL) $(HEADERS) $(LOCAL_INCLUDE)

clean :
	- $(RM) libmaps.a $(OBJECTS) $(SOURCES) $(HEADERS)

tar:
	$(CO) Makefile $(SOURCES) $(HEADERS)
	$(TAR) cvf $(TARFILE) Makefile $(SOURCES) $(HEADERS)
	$(COMPRESS) $(TARFILE)

# interactive tests
mtest : mapx.c mapx.h maps.c maps.h
	$(CC) $(DEBUG_FLAGS) -DMTEST -o mtest mapx.c maps.c $(CLIBS)

gtest : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) $(DEBUG_FLAGS) -DGTEST -o gtest grids.c mapx.c maps.c $(CLIBS)

# performance tests
mpmon : mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DMPMON -o mpmon mapx.c $(CLIBS)

gpmon : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DGPMON -o gpmon grids.c mapx.c $(CLIBS)

# accuracy tests
macct : maps.c maps.h mapx.c mapx.h
	$(CC) -O -DMACCT -o macct maps.c mapx.c $(CLIBS)

cdb.o:		cdb.h cdb_byteswap.h
mapx.o:		mapx.h
grids.o:	grids.h mapx.h
maps.o:		mapx.h grids.h cdb.h maps.h

.SUFFIXES : .c,v .h,v

.c,v.o :
	$(CO) $<
	$(CC) $(CFLAGS) -c $*.c
	- $(RM) $*.c

.c,v.c :
	$(CO) $<

.h,v.h :
	$(CO) $<
