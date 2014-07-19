#
# $Id: Makefile,v 1.1 2001-01-02 16:54:13 soliday Exp $
#
#

TOP=../../..
include $(TOP)/configure/CONFIG

SHARED_LIBRARIES = YES

LIBRARY = sddsdatamodule
LIBSRCS = sddsdatamodule.c
SHRLIB_PREFIX=

PYTHON_PREFIX = $(shell python -c "import sys; print sys.prefix")
PYTHON_EXEC_PREFIX = $(shell python -c "import sys; print sys.exec_prefix")
PYTHON_VERSION = $(shell python -c "import sys; print sys.version[:3]")
PYTHON_INCLUDE = $(PYTHON_PREFIX)/include/python$(PYTHON_VERSION)
PYTHON_EXEC_INCLUDE = $(PYTHON_EXEC_PREFIX)/include/python$(PYTHON_VERSION)

USR_CFLAGS = -I$(PYTHON_INCLUDE) -I$(PYTHON_EXEC_INCLUDE) -I$(EPICS_EXTENSIONS)/src/SDDS/lzma

LIB_SYS_LIBS_DEFAULT = z
LIB_SYS_LIBS_solaris = -nil-
LIB_SYS_LIBS_WIN32 = -nil-

LIB_LIBS = SDDS1 mdblib lzma
SDDS1_DIR = $(INSTALL_LIB)
mdblib_DIR = $(INSTALL_LIB)
lzma_DIR = $(INSTALL_LIB)

ifeq ($(EPICS_HOST_ARCH),win32-x86)
USR_CFLAGS = -Ic:/Python$(PYTHON)_32bit/include -I$(EPICS_EXTENSIONS)/src/SDDS/lzma
LIB_LIBS += python$(PYTHON)
python$(PYTHON)_DIR = C:/Python$(PYTHON)_32bit/libs
endif
ifeq ($(EPICS_HOST_ARCH),windows-x64)
USR_CFLAGS = -Ic:/Python$(PYTHON)_64bit/include -I$(EPICS_EXTENSIONS)/src/SDDS/lzma
LIB_LIBS += python$(PYTHON)
python$(PYTHON)_DIR = C:/Python$(PYTHON)_64bit/libs
endif

include $(TOP)/configure/RULES

