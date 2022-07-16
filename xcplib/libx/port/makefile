#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xcplibx:port/makefile	1.2.1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.	  

#
# makefile for libx/port
#
#
# The macro PROF is null by default, causing profiled object to be maintained.
# If profiled object is not desired, the reassignment PROF=@# should appear in
# the make command line.
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O -DMERGE
PROF=
NONPROF=
DEFLIST=
SDEFLIST=

all:	prof nonprof 

nonprof:
	$(NONPROF)cd sys; make -e $(IGN) nonprof PROF=@#
prof:
	$(PROF)cd sys; make -e $(IGN) prof NONPROF=@#
	
clean:

clobber: clean
	-rm -f */*.o
	-rm -f */*.p
