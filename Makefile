#========================================================================
# Makefile for mapx library
#
# 11-Feb-1993 K.Knowles 303-492-0644  knowlesk@kryos.colorado.edu
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 1993-2004 University of Colorado
#========================================================================
RCSID = $Header: /tmp_mnt/FILES/mapx/Makefile,v 1.60 2008-03-14 16:04:30 brodzik Exp $

#------------------------------------------------------------------------
# configuration section
#
#	installation directories
#
TOPDIR = $(HOME)
LIBDIR = $(TOPDIR)/lib
MAPDIR = $(LIBDIR)/maps
INCDIR = $(TOPDIR)/include
BINDIR = $(TOPDIR)/bin
UTESTDIR = unit_test
#
#	commands
#
SHELL = /bin/sh
CC = cc
AR = ar
RANLIB = touch
# Uncomment the next line for MAC OSX
#RANLIB = ranlib
CO = co
MAKEDEPEND = makedepend
INSTALL = cp -f -p
MKDIR = mkdir -p
CP = cp -f -p
RM = rm -f
TAR = tar
COMPRESS = gzip

#
#	archive file name
#
TARFILE = mapx.tar

#
#	debug or optimization settings
#
#	on least significant byte first machines (Intel, Vax)
#	add -DLSB1ST option to enable byteswapping of cdb files
#	for other architectures (Sun, SGI, HP, etc.) do _not_ use
#	the -DLSB1ST flag

CONFIG_CFLAGS = -O -DLSB1ST -fPIC 

#CONFIG_CFLAGS = -O -DLSB1ST
#CONFIG_CFLAGS = -DDEBUG -g
#CONFIG_CFLAGS = -DDEBUG -g -DLSB1ST
#CONFIG_CFLAGS = -O -Wall -DLSB1ST

#
#	system libraries
#
SYSLIBS = -lm

#
# end configuration section
#------------------------------------------------------------------------

CFLAGS = -I$(INCDIR) $(CONFIG_CFLAGS)
LIBS = -L$(LIBDIR) -lmapx $(SYSLIBS)
DEPEND_LIBS = $(LIBDIR)/libmapx.a

PROJECTION_SRCS = polar_stereographic.c orthographic.c cylindrical_equal_area.c \
mercator.c mollweide.c cylindrical_equidistant.c sinusoidal.c \
lambert_conic_conformal.c interupted_homolosine_equal_area.c \
albers_conic_equal_area.c azimuthal_equal_area.c \
integerized_sinusoidal.c \
transverse_mercator.c universal_transverse_mercator.c

PROJECTION_OBJS = polar_stereographic.o orthographic.o \
cylindrical_equal_area.o \
mercator.o mollweide.o cylindrical_equidistant.o sinusoidal.o \
lambert_conic_conformal.o interupted_homolosine_equal_area.o \
albers_conic_equal_area.o azimuthal_equal_area.o \
integerized_sinusoidal.o \
transverse_mercator.o universal_transverse_mercator.o

MAPX_SRCS = mapx.c grids.c cdb.c maps.c keyval.c grid_io.c $(PROJECTION_SRCS)
MAPX_HDRS = mapx.h grids.h cdb.h maps.h cdb_byteswap.h keyval.h grid_io.h
MAPX_OBJS = mapx.o grids.o cdb.o maps.o keyval.o grid_io.o $(PROJECTION_OBJS)

MODELS_SRCS = smodel.c pmodel.c svd.c lud.c matrix.c matrix_io.c
MODELS_OBJS = smodel.o pmodel.o svd.o lud.o matrix.o matrix_io.o
MODELS_HDRS = smodel.h pmodel.h svd.h lud.h matrix.h matrix_io.h

GCTP_SRCS = isinfor.c isininv.c report.c cproj.c
GCTP_OBJS = isinfor.o isininv.o report.o cproj.o
GCTP_HDRS = isin.h cproj.h proj.h

SRCS = $(MAPX_SRCS) $(MODELS_SRCS) $(GCTP_SRCS)
HDRS = define.h byteswap.h $(MAPX_HDRS) $(MODELS_HDRS) $(GCTP_HDRS)
OBJS = $(MAPX_OBJS) $(MODELS_OBJS) $(GCTP_OBJS)

all : libmapx.a install

allall: cleanall all appall testall

appall : gridloc regrid resamp irregrid ungrid \
	 cdb_edit cdb_list wdbtocdb mapenum

testall : xytest mtest gtest crtest macct gacct

libmapx.a : $(OBJS)
	$(AR) ruv libmapx.a $(OBJS)
	$(RANLIB) libmapx.a

install : libmapx.a $(HDRS)
	$(MKDIR) $(LIBDIR) $(INCDIR) $(BINDIR) $(MAPDIR)
	$(INSTALL) libmapx.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

cleanall : clean cleanexes
	$(RM) *.o
	$(RM) $(TARFILE).gz 

clean :
	- $(RM) libmapx.a $(OBJS)

cleanexes :
	- $(RM) cdb_edit cdb_list gacct gpmon gridloc gtest crtest irregrid \
		macct mapenum mpmon mtest regrid resamp wdbtocdb xytest ungrid

tar :
	$(RM) $(TARFILE).gz 
	$(TAR) cvf $(TARFILE) \
		README COPYING INSTALL\
		Makefile ppgc.html mprojex.gif coordef.gif \
		regrid.c resamp.c irregrid.c ungrid.c \
		cdb_edit.mpp cdb_edit.c cdb_list.c wdbtocdb.c wdbpltc.c \
		mapenum.c gridloc.c \
		$(SRCS) $(HDRS) $(UTESTDIR)/*.pl \
		$(UTESTDIR)/other/other* \
		$(UTESTDIR)/snyder/snyder* \
		$(UTESTDIR)/tilecalc/tilecalc* \
		$(UTESTDIR)/sgi/sgi* \
		$(UTESTDIR)/linux/linux* \
	 	$(UTESTDIR)/linuxhp/linuxhp*
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
resamp: resamp.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o resamp resamp.o $(LIBS)
	$(INSTALL) resamp $(BINDIR)
irregrid: irregrid.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o irregrid irregrid.o $(LIBS)
	$(INSTALL) irregrid $(BINDIR)
ungrid: ungrid.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o ungrid ungrid.o $(LIBS)
	$(INSTALL) ungrid $(BINDIR)
cdb_edit: cdb_edit.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o cdb_edit cdb_edit.o $(LIBS)
	$(INSTALL) cdb_edit $(BINDIR)
	$(INSTALL) cdb_edit.mpp $(MAPDIR)
cdb_list: cdb_list.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o cdb_list cdb_list.o $(LIBS)
	$(INSTALL) cdb_list $(BINDIR)
wdbtocdb: wdbtocdb.o wdbpltc.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o wdbtocdb wdbtocdb.o wdbpltc.o $(LIBS)
	$(INSTALL) wdbtocdb $(BINDIR)
mapenum: mapenum.o $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o mapenum mapenum.o $(LIBS)
	$(INSTALL) mapenum $(BINDIR)
#
#------------------------------------------------------------------------
# interactive tests
#
xytest : mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DXYTEST -o xytest mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) mapx.o
	$(INSTALL) xytest $(BINDIR)

mtest : mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DMTEST -o mtest mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) mapx.o
	$(INSTALL) mtest $(BINDIR)

gtest : grids.c grids.h mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DGTEST -o gtest grids.c mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) grids.o
	$(INSTALL) gtest $(BINDIR)

crtest : grids.c grids.h mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DCRTEST -o crtest grids.c mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) grids.o
	$(INSTALL) crtest $(BINDIR)
#
#------------------------------------------------------------------------
# performance tests
#
mpmon : mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -p -DMPMON -o mpmon mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) mapx.o
	$(INSTALL) mpmon $(BINDIR)

gpmon : grids.c grids.h mapx.c mapx.h maps.c maps.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -p -DGPMON -o gpmon grids.c mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) grids.o
	$(INSTALL) gpmon $(BINDIR)
#
#------------------------------------------------------------------------
# accuracy tests
#
macct : maps.c maps.h mapx.c mapx.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DMACCT -o macct mapx.c maps.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) mapx.o
	$(INSTALL) macct $(BINDIR)

gacct : grids.c maps.c maps.h mapx.c mapx.h keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS)
	$(CC) $(CFLAGS) -DGACCT -o gacct grids.c maps.c mapx.c keyval.o \
		$(PROJECTION_OBJS) $(GCTP_OBJS) $(SYSLIBS)
	$(RM) grids.o
	$(INSTALL) gacct $(BINDIR)
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

