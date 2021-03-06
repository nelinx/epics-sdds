#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
# National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
# Operator of Los Alamos National Laboratory.
# This file is distributed subject to a Software License Agreement found
# in the file LICENSE that is included with this distribution. 
#*************************************************************************
#
#
# High-level makefile for building SDDS
# $Log: not supported by cvs2svn $
# Revision 1.38  2012/03/22 19:33:24  soliday
# Modified because xlslib and gsl are GPL licenses and cannot be distributed
# with our software.
#
# Revision 1.37  2012/01/30 19:21:49  soliday
# Added the xlslib directory.
#
# Revision 1.36  2009/04/24 18:27:25  soliday
# Added the gsl subdirectory.
#
# Revision 1.35  2008/06/11 19:11:19  soliday
# Updated so that it will build the gd library on solaris-x86_64 systems
# no matter what.
#
# Revision 1.34  2007/10/22 20:56:45  soliday
# Changed the build order so that rpns/code is now built before namelist.
#
# Revision 1.33  2007/10/03 17:00:10  shang
# added 2d_interpolate
#
# Revision 1.32  2007/04/17 17:24:33  soliday
# Updated to better detect and use png, gd, and tiff system libraries.
#
# Revision 1.31  2006/11/09 20:15:21  soliday
# Fixed an issue with darwin-ppc
#
# Revision 1.30  2006/10/19 17:55:39  soliday
# Updated to work with linux-x86_64.
#
# Revision 1.29  2006/04/11 15:49:50  soliday
# Updated to work better under various build settings on Linux.
#
# Revision 1.28  2006/04/10 21:24:45  soliday
# Updated to always use the included version of tiff.
#
# Revision 1.27  2006/02/01 17:39:45  soliday
# Added levmar directory.
#
# Revision 1.26  2005/03/24 16:49:20  soliday
# Linux systems will now compile the gd library because not all linux
# systems have gd version 2 already installed.
#
# Revision 1.25  2004/02/27 22:37:52  soliday
# Added support for linux-athlon
#
# Revision 1.24  2004/01/06 21:13:55  soliday
# Improved the method for locating existing tiff library and headers.
#
# Revision 1.23  2003/09/17 19:24:21  soliday
# zlib is now built for vxWorks and all WIN32 compilers.
#
# Revision 1.22  2003/07/24 20:14:28  soliday
# Fixed for WIN32
#
# Revision 1.21  2003/07/23 20:54:04  soliday
# Modified makefile to work with base version 3.13 and 3.14
#
# Revision 1.20  2003/07/22 20:01:15  soliday
# Added support for Kylix.
#
# Revision 1.19  2002/09/26 15:23:59  soliday
# The tiff library does not have to be built on OSX if it already exists
#
# Revision 1.18  2002/09/19 16:36:36  soliday
# Added the tiff library
#
# Revision 1.17  2002/08/14 15:29:44  soliday
# Added Open License
#
# Revision 1.16  2001/08/09 18:25:23  soliday
# Added utils directory.
#
# Revision 1.15  2001/05/31 19:38:10  soliday
# Fixed a problem with Linux.
#
# Revision 1.14  2001/03/23 21:31:07  soliday
# Changed WIN32 so that it uses the new winMotifDriver.
#
# Revision 1.13  2000/12/07 21:59:53  jba
# Changes for building with base R3.14.
#
# Revision 1.12  2000/11/01 17:22:16  soliday
# Added zlib.
#
# Revision 1.11  2000/10/31 15:34:02  soliday
# Added png
#
# Revision 1.10  2000/10/04 22:31:48  soliday
# Linux no longer makes the gd library. I am counting on the gd library to
# already be installed in /usr/lib.
#
# Revision 1.9  2000/04/17 19:23:57  soliday
# Added the exceedWrapper directory.
#
# Revision 1.8  2000/04/11 16:32:51  soliday
# This Makefile removes all the circular builds that we had to do before.
#
# Revision 1.7  1999/09/16 22:11:34  soliday
# Modified the way WIN32 libraries are built.
#
# Revision 1.6  1999/07/23 16:48:25  soliday
# Added fdlibm for WIN32
#
# Revision 1.5  1999/05/04 19:24:03  jba
# Moved gd from oag.
#
# Revision 1.4  1999/04/15 19:55:29  jba
# Added meschach directory.
#
# Revision 1.3  1999/04/14 14:04:36  borland
# Works with new build system.  Fixed some missing prototypes and function type mismatches.
#
# Revision 1.2  1995/09/18 16:09:02  saunders
# Added auto installation of headers into extensions/include.
#
# Revision 1.1  1995/09/05  21:05:19  saunders
# First test release of the SDDS1.5 package.
#
#

TOP=../..
ifeq (0, $(words $(notdir $(wildcard $(TOP)/configure))))
include $(TOP)/config/CONFIG_EXTENSIONS
else
include $(TOP)/configure/CONFIG
endif

BUILD_PSEUDOINVERSE=1

DIRS = 	include meschach 
ifeq ($(EPICS_HOST_ARCH),win32-x86)
BUILD_FDLIBM=1
BUILD_WINMOTIFDRIVER=1
endif
ifeq ($(EPICS_HOST_ARCH),windows-x64)
BUILD_FDLIBM=1
BUILD_WINMOTIFDRIVER=1
endif
ifeq ($(EPICS_HOST_ARCH),win32-x86-mingw)
BUILD_WINMOTIFDRIVER=1
BUILD_PSEUDOINVERSE=1
endif
ifeq ($(EPICS_HOST_ARCH),windows-x64-mingw)
BUILD_WINMOTIFDRIVER=1
BUILD_PSEUDOINVERSE=1
endif


ifdef BUILD_FDLIBM
DIRS += fdlibm
endif


#Look for private version of XLSLIB used here at APS
#xlslib can be found at http://sourceforge.net/projects/xlslib/files/
#We use it in sdds2spreadsheet for creating Excel files.
XLS = $(words $(wildcard xlslib))
ifeq ($(XLS), 1)
ifneq ($(HOST_ARCH),linux-ppc64-xl)
DIRS += xlslib
endif
endif

DIRS += zlib lzma


#Look for private version of GSL used here at APS
#This is often available as a system package for linux.
#The source code can be found at ftp://ftp.gnu.org/gnu/gsl/
#It is required for Pelegant
GSL = $(words $(wildcard gsl))
ifeq ($(GSL), 1)
DIRS += gsl
endif

DIRS += mdblib mdbmth rpns/code namelist \
	SDDSlib fftpack matlib mdbcommon utils 2d_interpolate

PNG = $(words $(notdir $(wildcard \
/usr/lib/libpng.a \
/usr/lib/libpng.so \
/usr/lib64/libpng.a \
/usr/lib64/libpng.so \
/usr/lib/i386-linux-gnu/libpng.a \
/usr/lib/i386-linux-gnu/libpng.so \
/usr/lib/x86_64-linux-gnu/libpng.a \
/usr/lib/x86_64-linux-gnu/libpng.so \
/lib/libpng.a \
/lib/libpng.so \
/lib64/libpng.a \
/lib64/libpng.so \
/sw/lib/libpng.a \
/sw/lib/libpng.dylib \
/opt/local/lib/libpng.a \
/opt/local/lib/libpng.dylib \
/usr/sfw/lib/libpng.a \
/usr/sfw/lib/libpng.so )))
PNGINC = $(words $(notdir $(wildcard \
/usr/include/png.h \
/include/png.h \
/sw/include/png.h \
/opt/local/include/png.h \
/usr/sfw/include/png.h )))

#Here at the APS /opt/local/lib/libgd.a exists but is not 64 bit
#and sddsplot expects gd and png to both built locally if gd is.
ifeq ($(HOST_ARCH),solaris-x86_64)
PNG = 0
endif
ifeq ($(HOST_ARCH),linux-ppc64)
PNG = 0
endif
ifeq ($(HOST_ARCH),linux-ppc64-xl)
PNG = 0
endif

ifeq ($(PNG), 0)
DIRS += png
else
ifeq ($(PNGINC), 0)
DIRS += png
endif
endif

GD = $(words $(notdir $(wildcard \
/usr/lib/libgd.a \
/usr/lib/libgd.so \
/usr/lib64/libgd.a \
/usr/lib64/libgd.so \
/usr/lib/i386-linux-gnu/libgd.a \
/usr/lib/i386-linux-gnu/libgd.so \
/usr/lib/x86_64-linux-gnu/libgd.a \
/usr/lib/x86_64-linux-gnu/libgd.so \
/lib/libgd.a \
/lib/libgd.so \
/lib64/libgd.a \
/lib64/libgd.so \
/sw/lib/libgd.a \
/sw/lib/libgd.dylib \
/opt/local/lib/libgd.a \
/opt/local/lib/libgd.dylib \
/opt/sfw/lib/libgd.a \
/opt/sfw/lib/libgd.so \
/opt/local/lib/libgd.so )))
GDINC = $(words $(notdir $(wildcard \
/usr/include/gd.h \
/include/gd.h \
/sw/include/gd.h \
/opt/sfw/include/gd.h \
/opt/local/include/gd.h )))

#Here at the APS /opt/local/lib/libgd.a exists but is not 64 bit
ifeq ($(HOST_ARCH),solaris-x86_64)
GD = 0
endif
ifeq ($(HOST_ARCH),linux-ppc64)
GD = 0
endif
ifeq ($(HOST_ARCH),linux-ppc64-xl)
GD = 0
endif

ifeq ($(GD), 0)
DIRS += gd
else
ifeq ($(GDINC), 0)
DIRS += gd
endif
endif

TIFF = $(words $(notdir $(wildcard \
/usr/lib/libtiff.a \
/usr/lib/libtiff.so \
/usr/lib64/libtiff.a \
/usr/lib64/libtiff.so \
/usr/lib/i386-linux-gnu/libtiff.a \
/usr/lib/i386-linux-gnu/libtiff.so \
/usr/lib/x86_64-linux-gnu/libtiff.a \
/usr/lib/x86_64-linux-gnu/libtiff.so \
/lib/libtiff.a \
/lib/libtiff.so \
/lib64/libtiff.a \
/lib64/libtiff.so \
/sw/lib/libtiff.a \
/sw/lib/libtiff.dylib \
/opt/local/lib/libtiff.a \
/opt/local/lib/libtiff.dylib \
/usr/sfw/lib/libtiff.a \
/usr/sfw/lib/libtiff.so )))
TIFFINC = $(words $(notdir $(wildcard \
/usr/include/tiffio.h \
/include/tiffio.h \
/sw/include/tiffio.h \
/opt/local/include/tiffio.h \
/usr/sfw/include/tiffio.h )))

ifeq ($(HOST_ARCH),linux-ppc64)
TIFF = 0
endif
ifeq ($(HOST_ARCH),linux-ppc64-xl)
TIFF = 0
endif

ifeq ($(TIFF), 0)
DIRS += tiff
else
ifeq ($(TIFFINC), 0)
DIRS += tiff
endif
endif

DIRS += SDDSaps 
ifneq ($(HOST_ARCH),linux-ppc64-xl)
DIRS += SDDSaps/sddsplots
ifdef BUILD_WINMOTIFDRIVER
DIRS += SDDSaps/sddsplots/winMotifDriver
else
DIRS += SDDSaps/sddsplots/motifDriver
endif
endif

ifeq ($(BUILD_PSEUDOINVERSE),1)
DIRS += SDDSaps/pseudoInverse
endif
ifneq ($(HOST_ARCH),linux-ppc64-xl)
DIRS += SDDSaps/sddscontours
endif
#Look for private version of LEVMAR used here at APS
LEVMAR = $(words $(wildcard levmar))
ifeq ($(LEVMAR), 1)
DIRS += levmar
endif

ifeq (0, $(words $(notdir $(wildcard $(TOP)/configure))))
include $(TOP)/config/RULES_DIRS
else
include $(TOP)/configure/RULES_DIRS
endif
