#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@sastrugi.colorado.edu
# National Snow & Ice Data Center, University of Colorado, Boulder
#========================================================================
RCSID = $Header: /tmp_mnt/FILES/mapx/Makefile,v 1.25 1994-07-14 14:56:24 swick Exp $

#------------------------------------------------------------------------
# configuration section
#
#	installation directories
#
TOPDIR = /usr/local
LIBDIR = $(TOPDIR)/lib
MAPDIR = $(LIBDIR)/maps
INCDIR = $(TOPDIR)/include
BINDIR = $(TOPDIR)/bin
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
#	on least significant byte first machines (Intel, Vax)
#	add -DLSB1ST option to enable byteswapping of cdb files
#
CONFIG_CFLAGS = -O
#CONFIG_CFLAGS = -DDEBUG -g

#
#	system libraries
#
SYSLIBS = -lm

#
# end configuration section
#------------------------------------------------------------------------

CFLAGS = -I$(INCDIR) $(CONFIG_CFLAGS)
LIBS = -L$(LIBDIR) -lmaps $(SYSLIBS)

SRCS = mapx.c grids.c cdb.c maps.c cdb_edit.c cdb_list.c wdbtocdb.c wdbpltc.c
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
	- $(CO) Makefile ppgc.ps regrid.c cdb_edit.mpp $(SRCS) $(HDRS)
	$(TAR) cvf $(TARFILE) Makefile ppgc.ps regrid.c $(SRCS) $(HDRS)
	$(COMPRESS) $(TARFILE)

depend :
	- $(CO) $(SRCS) $(HDRS)
	$(MAKEDEPEND) -I$(INCDIR) -- $(CFLAGS) -- $(SRCS)

#------------------------------------------------------------------------
# applications
#
regrid: regrid.o $(LIBDIR)/libmaps.a
	$(CC) $(CFLAGS) -o regrid regrid.o $(LIBS) -lmodels
	$(INSTALL) regrid $(BINDIR)
#
#------------------------------------------------------------------------
# interactive tests
#
mtest : mapx.c mapx.h maps.c maps.h
	$(CC) $(CFLAGS) -DMTEST -o mtest mapx.c maps.c $(LIBS)

gtest : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) $(CFLAGS) -DGTEST -o gtest grids.c mapx.c maps.c $(LIBS)
#
#------------------------------------------------------------------------
# performance tests
#
mpmon : mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DMPMON -o mpmon mapx.c $(LIBS)

gpmon : grids.c grids.h mapx.c mapx.h maps.c maps.h
	$(CC) -O -p -DGPMON -o gpmon grids.c mapx.c $(LIBS)
#
#------------------------------------------------------------------------
# accuracy tests
#
macct : maps.c maps.h mapx.c mapx.h
	$(CC) -O -DMACCT -o macct maps.c mapx.c $(LIBS)
#
#------------------------------------------------------------------------
# coastlines database aplications
#
cdb_edit: cdb_edit.o $(LIBDIR)/libmaps.a
	$(CC) -o cdb_edit cdb_edit.o $(LIBS)
	$(INSTALL) cdb_edit $(BINDIR)
cdb_edit.mpp: cdb_edit.mpp,v
	$(CO) cdb_edit.mpp
	$(INSTALL) cdb_edit.mpp $(MAPDIR)
cdb_list: cdb_list.o $(LIBDIR)/libmaps.a
	$(CC) -o cdb_list cdb_list.o $(LIBS)
	$(INSTALL) cdb_list $(BINDIR)
wdbtocdb: wdbtocdb.o wdbpltc.o $(LIBDIR)/libmaps.a
	$(CC) -o wdbtocdb wdbtocdb.o wdbpltc.o $(LIBS)
	$(INSTALL) wdbtocdb $(BINDIR)
#
#------------------------------------------------------------------------

.SUFFIXES : .c,v .h,v

.c,v.o :
	$(CO) $<
	$(CC) $(CFLAGS) -c $*.c
	- $(RM) $*.c

.c,v.c :
	$(CO) $<

.h,v.h :
	$(CO) $<

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
maps.o: /usr/include/float.h /usr/local/include/define.h maps.h mapx.h
cdb_edit.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
cdb_edit.o: /usr/include/math.h /usr/include/svr4_math.h /usr/include/errno.h
cdb_edit.o: /usr/include/sys/errno.h /usr/include/ctype.h
cdb_edit.o: /usr/include/assert.h /usr/local/include/define.h cdb.h mapx.h
cdb_edit.o: cdb_byteswap.h /usr/local/include/byteswap.h
cdb_list.o: /usr/include/stdio.h /usr/include/math.h /usr/include/svr4_math.h
cdb_list.o: /usr/local/include/define.h cdb.h
wdbtocdb.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
wdbtocdb.o: /usr/include/assert.h /usr/include/math.h
wdbtocdb.o: /usr/include/svr4_math.h /usr/local/include/define.h mapx.h cdb.h
wdbtocdb.o: cdb_byteswap.h /usr/local/include/byteswap.h
wdbpltc.o: /usr/include/stdlib.h /usr/include/fcntl.h
wdbpltc.o: /usr/include/sys/types.h /usr/include/sys/fcntl.h
wdbpltc.o: /usr/include/sys/stat.h /usr/include/stdio.h /usr/include/errno.h
wdbpltc.o: /usr/include/sys/errno.h /usr/include/math.h
wdbpltc.o: /usr/include/svr4_math.h
