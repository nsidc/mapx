#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@sastrugi.colorado.edu
# National Snow & Ice Data Center, University of Colorado, Boulder
#========================================================================
RCSID = $Header: /tmp_mnt/FILES/mapx/Makefile,v 1.15 1994-04-08 10:49:17 knowles Exp $

#------------------------------------------------------------------------
# configuration section
#
#	installation directories
#
LIBDIR = /usr/local/lib
INCDIR = /usr/local/include

#
#	commands
#
SHELL = /bin/sh
CC = cc
AR = ar
RANLIB = touch
CO = co
MAKEDEPEND = makedepend
INSTALL = cp
RM = rm -f
TAR = tar
COMPRESS = compress

#
#	archive file name
#
TARFILE = maps.tar

#
#	debug or optimization settings
#
DEBUG_CFLAGS = -O
#DEBUG_CFLAGS = -DDEBUG -g

#
#	system libraries
#
SYSLIBS = -lm

#
# end configuration section
#------------------------------------------------------------------------

CFLAGS = -I$(INCDIR) $(DEBUG_CFLAGS)
LIBS = -L$(LIBDIR) -lmaps $(SYSLIBS)

SRCS = mapx.c grids.c cdb.c maps.c
HDRS = mapx.h grids.h cdb.h maps.h cdb_byteswap.h
OBJS = mapx.o grids.o cdb.o maps.o

all : libmaps.a install

libmaps.a : $(OBJS)
	$(AR) ruv libmaps.a $(OBJS)
	$(RANLIB) libmaps.a

install : libmaps.a $(HDRS)
	$(INSTALL) libmaps.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

clean :
	- $(RM) libmaps.a $(OBJS)

tar :
	$(CO) Makefile $(SRCS) $(HDRS)
	$(TAR) cvf $(TARFILE) Makefile $(SRCS) $(HDRS)
	$(COMPRESS) $(TARFILE)

# interactive tests
mtest : mapx.c mapx.h maps.c maps.h
	$(CC) $(CFLAGS) -DMTEST -o mtest mapx.c maps.c $(LIBS)

gtest : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) $(CFLAGS) -DGTEST -o gtest grids.c mapx.c maps.c $(LIBS)

# performance tests
mpmon : mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DMPMON -o mpmon mapx.c $(LIBS)

gpmon : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DGPMON -o gpmon grids.c mapx.c $(LIBS)

# accuracy tests
macct : maps.c maps.h mapx.c mapx.h
	$(CC) -O -DMACCT -o macct maps.c mapx.c $(LIBS)


.SUFFIXES : .c,v .h,v

.c,v.o :
	$(CO) $<
	$(CC) $(CFLAGS) -c $*.c
	- $(RM) $*.c

.c,v.c :
	$(CO) $<

.h,v.h :
	$(CO) $<

depend :
	$(CO) $(SRCS) $(HDRS)
	$(MAKEDEPEND) -I$(INCDIR) -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

mapx.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
mapx.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/ctype.h
mapx.o: /usr/include/math.h /usr/include/svr4_math.h
mapx.o: /usr/local/include/define.h maps.h mapx.h
grids.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
grids.o: /usr/local/include/define.h maps.h /usr/include/math.h
grids.o: /usr/include/svr4_math.h mapx.h grids.h
cdb.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
cdb.o: /usr/include/math.h /usr/include/svr4_math.h
cdb.o: /usr/local/include/define.h maps.h mapx.h cdb.h cdb_byteswap.h
cdb.o: /usr/local/include/byteswap.h
maps.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
maps.o: /usr/include/ctype.h /usr/include/math.h /usr/include/svr4_math.h
maps.o: /usr/local/include/define.h maps.h mapx.h
