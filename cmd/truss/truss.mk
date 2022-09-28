#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)truss:truss.mk	1.6.3.1"
#
# makefile for truss(1) command
#
# make		- make truss in local directory
# make install	- make truss and install in $(INSDIR)
# make lint	- check program consistency
# make clean	- as your mother told you
# make clobber	- make it squeaky clean

PROCISSUE = PROC Issue 1 Version 10

CC = cc
LD = ld
MCS = mcs

ROOT   =
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
INS = install

DEFINES =

CFLAGS = -O -I$(INC) $(DEFINES)

LDFLAGS = -s

HEADERS = pcontrol.h ioc.h ramdata.h proto.h systable.h print.h

SOURCES = main.c listopts.c ipc.c actions.c expound.c incdec.c \
	codes.c print.c pcontrol.c ramdata.c systable.c procset.c xstat.c

OBJECTS = main.o listopts.o ipc.o actions.o expound.o incdec.o \
	codes.o print.o pcontrol.o ramdata.o systable.o procset.o xstat.o

truss:	$(OBJECTS)
	$(CC) -o truss $(OBJECTS) $(SHLIBS) $(LDFLAGS)
#	$(MCS) -d truss
#	$(MCS) -a "@(#)/bin/truss $(PROCISSUE) `date +%m/%d/%y`" truss

actions.o:	pcontrol.h ramdata.h systable.h print.h proto.h
codes.o:	pcontrol.h ioc.h ramdata.h proto.h
expound.o:	pcontrol.h ramdata.h systable.h proto.h
incdec.o:	pcontrol.h ramdata.h proto.h
ipc.o:		pcontrol.h ramdata.h proto.h
listopts.o:	pcontrol.h ramdata.h systable.h proto.h
main.o:		pcontrol.h ramdata.h proto.h
pcontrol.o:	pcontrol.h ramdata.h proto.h
print.o:	pcontrol.h print.h ramdata.h proto.h
procset.o:	pcontrol.h ramdata.h proto.h
systable.o:	pcontrol.h ramdata.h systable.h print.h proto.h
xstat.o:	pcontrol.h ramdata.h proto.h
ramdata.o:	pcontrol.h ramdata.h proto.h


install:	truss
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin truss

lint:
	lint $(CFLAGS) $(SOURCES)

clean:
	rm -f *.o

clobber:	clean
	rm -f truss
