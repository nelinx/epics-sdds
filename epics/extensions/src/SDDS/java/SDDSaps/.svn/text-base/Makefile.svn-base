#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
# National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
# Operator of Los Alamos National Laboratory.
# This file is distributed subject to a Software License Agreement found
# in the file LICENSE that is included with this distribution. 
#*************************************************************************
#
# makefile for Java SDDS command line applications
# $Log: not supported by cvs2svn $
# Revision 1.9  2002/08/14 19:32:52  soliday
# Added Open License
#
# Revision 1.8  2002/06/24 16:01:08  soliday
# Added JAVADOC option
#
# Revision 1.7  2002/06/24 15:43:45  soliday
# Changed the JARCMD option
#
# Revision 1.6  2001/06/05 18:49:28  soliday
# Changed package names.
#
# Revision 1.5  2000/05/10 22:05:10  soliday
# Fixed a bug.
#
# Revision 1.4  2000/05/10 17:37:17  soliday
# Revamped makefile so that the class files are not in a package.
#
# Revision 1.3  2000/05/09 21:13:39  soliday
# Modified the method in which jar files are created.
#
# Revision 1.2  2000/05/03 16:52:22  soliday
# Modifed the manner in which documentation is created.
#
# Revision 1.1  2000/05/01 14:32:29  soliday
# First version.
#
#
#

TOP = ../../../..
include $(TOP)/configure/CONFIG

JAR = SDDSaps.jar
JAR_PACKAGES = SDDS/java/SDDSaps SDDS/java/SDDSplot SDDS/java/SDDS
JAVACFLAGS += -O

JAVA = sddsconvert.java sddsquery.java sddsplot.java

include $(TOP)/configure/RULES

