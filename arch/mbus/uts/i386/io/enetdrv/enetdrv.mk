#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright 1987, 1989 Intel Corporation
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied under the terms
#	of a license agreement or nondisclosure
#	agreement with Intel Corporation and may 
#	not be copied nor disclosed except in
#	accordance with the terms of that agreement.

#ident	"@(#)mbus:uts/i386/io/enetdrv/enetdrv.mk	1.3.1.1"

#	Makefile for i530 driver 

UNIXVER =	SVR40
INCRT =		../..
CFLAGS =	-O -I$(INCRT)  -DMB2 -D_KERNEL $(MORECPP)
LOCFLAGS =	-DEDL -DNS -UDEBUG -I../..  -D$(UNIXVER) $(CFLAGS)
COMFILES =	$(CONF)/pack.d/i530/Driver.o $(CONF)/pack.d/edlina/Driver.o

SYS =		$(INCRT)/sys/enet.h \
		$(INCRT)/sys/lcidef.h \
		$(INCRT)/sys/enetuser.h \
		$(INCRT)/sys/edlina.h

ENETSRC =	enetm.c\
		enetl.c\
		enetrdwri.c\
		enetutil.c\
		i530intr.c\
		i530hwdep.c\
		lci.c\
 		mhost.c\
		iNA961.c 

EDLINASRC =	edlina.c

ENETOBJS =	$(ENETSRC:.c=.o)

EDLINAOBJS =	$(EDLINASRC:.c=.o)

all:	$(COMFILES) $(FRC)
	-@echo "\n****** i530 Driver built ******"

$(CONF)/pack.d/i530/Driver.o:	$(ENETOBJS)
	[ -d $(CONF)/pack.d/i530 ] || mkdir $(CONF)/pack.d/i530
	$(LD) -r -o $@ $(ENETOBJS)

$(CONF)/pack.d/edlina/Driver.o:	$(EDLINAOBJS)
	[ -d $(CONF)/pack.d/edlina ] || mkdir $(CONF)/pack.d/edlina
	$(LD) -r -o $@ $(EDLINAOBJS)

enetm.o: enetm.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

enetl.o: enetl.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

i530hwdep.o: i530hwdep.c \
	$(INCRT)/sys/lcidef.h \
	$(INCRT)/sys/enet.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

i530intr.o: i530intr.c \
	$(INCRT)/sys/lcidef.h \
	$(INCRT)/sys/enet.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

enetrdwri.o: enetrdwri.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

enetutil.o: enetutil.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iNA961.o: iNA961.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

mhost.o: mhost.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/lcidef.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

lci.o: lci.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/lcidef.h \
	$(INCRT)/sys/enetuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

edlina.o: edlina.c \
	$(INCRT)/sys/enet.h \
	$(INCRT)/sys/edlina.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMFILES) 

FRC:
