#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@kryos.colorado.edu
#========================================================================
SHELL = /bin/sh
LOCAL_LIB = /usr/local/lib
LOCAL_INCLUDE = /usr/local/include
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
CLIBS = -lc_s -lm -lmalloc
CFLAGS = -I. $(DEBUG_FLAGS)
SOURCES = mapx.c grids.c cdb.c maps.c
HEADERS = mapx.h grids.h cdb.h maps.h
OBJECTS = mapx.o grids.o cdb.o maps.o

all : libmaps.a install clean

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
	- $(RM) $(SOURCES) $(HEADERS)

# interactive tests
mtest : mapx.c mapx.h
	$(CC) $(DEBUG_FLAGS) -DMTEST -o mtest mapx.c $(CLIBS)

gtest : grids.c grids.h mapx.c mapx.h
	$(CC) $(DEBUG_FLAGS) -DGTEST -o gtest grids.c mapx.c $(CLIBS)

# performance tests
mpmon : mapx.c mapx.h
	$(CC) -O -p -DMPMON -o mpmon mapx.c -lm -lmalloc

gpmon : grids.c grids.h mapx.c mapx.h
	$(CC) -O -p -DGPMON -o gpmon grids.c mapx.c -lm -lmalloc

# accuracy tests
macct : maps.c maps.h mapx.c mapx.h
	$(CC) -O -DMACCT -o macct maps.c mapx.c $(CLIBS)

cdb.o:		$(LOCAL_INCLUDE)/cdb.h
mapx.o:		$(LOCAL_INCLUDE)/mapx.h
grids.o:	$(LOCAL_INCLUDE)/grids.h $(LOCAL_INCLUDE)/mapx.h
maps.o:		$(LOCAL_INCLUDE)/mapx.h $(LOCAL_INCLUDE)/grids.h \
		$(LOCAL_INCLUDE)/cdb.h $(LOCAL_INCLUDE)/maps.h

.SUFFIXES : .c,v .h,v

.c,v.o :
	$(CO) $<
	$(CC) $(CFLAGS) -c $*.c
	- $(RM) $*.c

.c,v.c :
	$(CO) $<

.h,v.h :
	$(CO) $<
