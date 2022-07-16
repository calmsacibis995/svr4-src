#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xcplibx:libx/port/sys/makefile	1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.	 

#
# makefile for libx/port/sys
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O -DMERGE
PROF=
NONPROF=
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
DEFLIST=
SDEFLIST=

OBJECTS=sdget.o lock.o

POBJECTS=sdget.p lock.p

nonprof: $(OBJECTS)

prof: $(POBJECTS)

sdget.o sdget.p: $(INCSYS)/sys/sd.h $(INC)/errno.h
lock.o lock.p: $(INCSYS)/sys/lock.h

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p
