#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@kryos.colorado.edu
#========================================================================
SHELL = /bin/sh
LOCAL_LIBS = /usr/local/lib
LOCAL_INCLUDE = /usr/local/include
CC = /usr/bin/cc
AR = /usr/bin/ar
RANLIB = /bin/touch
MV = /bin/mv
RM = /bin/rm -f
DEBUG_FLAGS = -O
CLIBS = -lc_s -lm -lmalloc
CFLAGS = -I$(LOCAL_INCLUDE) $(DEBUG_FLAGS)
OBJS = mapx.o grids.o cdb.o maps.o

all : libmaps.a install clean

libmaps.a : $(OBJS)
	$(AR) ruv libmaps.a $(OBJS)
	$(RANLIB) libmaps.a

install : libmaps.a
	$(MV) libmaps.a $(LOCAL_LIBS)

clean :
	$(RM) $(OBJS)

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
macct : mapx.c mapx.h
	$(CC) -O -DMACCT -o macct mapx.c $(CLIBS)

cdb.o:		$(LOCAL_INCLUDE)/cdb.h
mapx.o:		$(LOCAL_INCLUDE)/mapx.h
grids.o:	$(LOCAL_INCLUDE)/grids.h $(LOCAL_INCLUDE)/mapx.h
maps.o:		$(LOCAL_INCLUDE)/mapx.h $(LOCAL_INCLUDE)/grids.h \
		$(LOCAL_INCLUDE)/cdb.h $(LOCAL_INCLUDE)/maps.h

