#========================================================================
# Makefile for maps library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowles@sastrugi.colorado.edu
# National Snow & Ice Data Center, University of Colorado, Boulder
#========================================================================
RCSID = $Header: /tmp_mnt/FILES/mapx/Makefile,v 1.37 1999-07-28 22:24:27 knowles Exp $

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
CP = cp
RM = rm -f
TAR = tar
COMPRESS = gzip

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
#CONFIG_CFLAGS = -O
CONFIG_CFLAGS = -DDEBUG -g

#
#	system libraries
#
SYSLIBS = -lm

#
# end configuration section
#------------------------------------------------------------------------

CFLAGS = -I$(INCDIR) $(CONFIG_CFLAGS)
LIBS = -L$(LIBDIR) -lmaps -lmodels $(SYSLIBS)
DEPEND_LIBS = $(LIBDIR)/libmaps.a $(LIBDIR)/libmodels.a

PROJECTION_SRCS = polar_stereographic.c orthographic.c cylindrical_equal_area.c \
mercator.c mollweide.c cylindrical_equidistant.c sinusoidal.c \
lambert_conic_conformal.c interupted_homolosine_equal_area.c \
albers_conic_equal_area.c azimuthal_equal_area.c 

PROJECTION_OBJS = polar_stereographic.o orthographic.o cylindrical_equal_area.o \
mercator.o mollweide.o cylindrical_equidistant.o sinusoidal.o \
lambert_conic_conformal.o interupted_homolosine_equal_area.o \
albers_conic_equal_area.o azimuthal_equal_area.o 

SRCS = mapx.c grids.c cdb.c maps.c $(PROJECTION_SRCS)
HDRS = mapx.h grids.h cdb.h maps.h cdb_byteswap.h define.h
OBJS = mapx.o grids.o cdb.o maps.o $(PROJECTION_OBJS)


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
	- $(CO) Makefile ppgc.ps regrid.c cdb_edit.mpp cdb_edit.c \
		cdb_list.c wdbtocdb.c wdbpltc.c mapenum.c gridloc.c \
		$(SRCS) $(HDRS)
	- $(CP) $(INCDIR)/define.h $(INCDIR)/byteswap.h .
	$(TAR) cvf $(TARFILE) define.h byteswap.h \
		Makefile ppgc.ps regrid.c cdb_edit.mpp cdb_edit.c \
                cdb_list.c wdbtocdb.c wdbpltc.c mapenum.c gridloc.c \
		$(SRCS) $(HDRS)
	$(COMPRESS) $(TARFILE)

depend :
	- $(CO) $(SRCS) $(HDRS)
	$(MAKEDEPEND) -I$(INCDIR) -- $(CFLAGS) -- $(SRCS)

#------------------------------------------------------------------------
# applications
#
gridloc: gridloc.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o gridloc gridloc.o $(LIBS)
	$(INSTALL) gridloc $(BINDIR)
regrid: regrid.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o regrid regrid.o $(LIBS)
	$(INSTALL) regrid $(BINDIR)
cdb_edit: cdb_edit.o $(DEPEND_LIBS)
	$(CC) -o cdb_edit cdb_edit.o $(LIBS)
	$(INSTALL) cdb_edit $(BINDIR)
	$(CP) cdb_edit.mpp $(MAPDIR)
cdb_list: cdb_list.o $(DEPEND_LIBS)
	$(CC) -o cdb_list cdb_list.o $(LIBS)
	$(INSTALL) cdb_list $(BINDIR)
wdbtocdb: wdbtocdb.o wdbpltc.o $(DEPEND_LIBS)
	$(CC) -o wdbtocdb wdbtocdb.o wdbpltc.o $(LIBS)
	$(INSTALL) wdbtocdb $(BINDIR)
mapenum: mapenum.o $(DEPEND_LIBS)
	$(CC) -o mapenum mapenum.o $(LIBS)
	$(INSTALL) mapenum $(BINDIR)
#
#------------------------------------------------------------------------
# interactive tests
#
mtest : mapx.c mapx.h maps.c maps.h $(PROJECTION_OBJS)
	$(CC) $(CFLAGS) -DMTEST -o mtest mapx.c maps.c $(PROJECTION_OBJS) $(SYSLIBS)

gtest : grids.c grids.h mapx.c mapx.h maps.c maps.h  $(PROJECTION_OBJS)
	$(CC) $(CFLAGS) -DGTEST -o gtest grids.c mapx.c maps.c $(PROJECTION_OBJS) $(SYSLIBS)
#
#------------------------------------------------------------------------
# performance tests
#
mpmon : mapx.c mapx.h maps.c maps.h $(PROJECTION_OBJS)
	$(CC) -O -p -DMPMON -o mpmon mapx.c maps.c $(PROJECTION_OBJS) $(SYSLIBS)

gpmon : grids.c grids.h mapx.c mapx.h maps.c maps.h $(PROJECTION_OBJS)
	$(CC) -O -p -DGPMON -o gpmon grids.c mapx.c maps.c $(PROJECTION_OBJS) $(SYSLIBS)
#
#------------------------------------------------------------------------
# accuracy tests
#
macct : maps.c maps.h mapx.c mapx.h $(PROJECTION_OBJS)
	$(CC) -O -DMACCT -o macct maps.c mapx.c $(PROJECTION_OBJS) $(SYSLIBS)
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

