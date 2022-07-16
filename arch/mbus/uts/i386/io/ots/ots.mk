#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988, 1989  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.
#

#ident	"@(#)mbus:uts/i386/io/ots/ots.mk	1.3"

#	Makefile for ots driver 

INCRT =		../..
DRIVER =	ots otsdg

CFLAGS =	-O -I$(INCRT) -DMB2 -D_KERNEL $(MORECPP)
LOCFLAGS =  -UDEBUG -I../.. $(CFLAGS)

COMFILES =	$(CONF)/pack.d/ots/Driver.o $(CONF)/pack.d/otsdg/Driver.o

SYS =		$(INCRT)/sys/ots.h \
		$(INCRT)/sys/otsuser.h\
		$(INCRT)/sys/otsprot.h\
		$(INCRT)/sys/otserror.h

OTSOBJS =	ots.o\
		iTLIrd.o\
		iTLIutils.o\
		iTLIwri.o\
		iMB2rd.o\
		iMB2utils.o\
		iMB2wri.o\
		otsintr.o\
		otsutils.o\
		otswri.o

OTSDGOBJS =	otsdg.o

all:	$(COMFILES) $(FRC)
	-@echo "\t****** OTS Drivers built ******"

$(CONF)/pack.d/otsdg/Driver.o:	$(OTSDGOBJS)
	[ -d $(CONF)/pack.d/otsdg ] || mkdir $(CONF)/pack.d/otsdg
	$(LD) -r -o $@ $(OTSDGOBJS)

$(CONF)/pack.d/ots/Driver.o:	$(OTSOBJS)
	[ -d $(CONF)/pack.d/ots ] || mkdir $(CONF)/pack.d/ots
	$(LD) -r -o $@ $(OTSOBJS)

ots.o: ots.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iTLIrd.o: iTLIrd.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iTLIwri.o: iTLIwri.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iTLIutils.o: iTLIutils.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iMB2rd.o: iMB2rd.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h\
	$(INCRT)/sys/otserror.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iMB2wri.o: iMB2wri.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h\
	$(INCRT)/sys/otserror.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

iMB2utils.o: iMB2utils.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

otsintr.o: otsintr.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h\
	$(INCRT)/sys/otsprot.h $(INCRT)/sys/otserror.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

otswri.o: otswri.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h\
	$(INCRT)/sys/otsprot.h $(INCRT)/sys/otserror.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

otsutils.o: otsutils.c \
	$(INCRT)/sys/ots.h $(INCRT)/sys/otsuser.h\
	$(INCRT)/sys/otsprot.h $(INCRT)/sys/otserror.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

otsdg.o: otsdg.c \
	$(INCRT)/sys/ots.h
	$(CC) $(LOCFLAGS) -c $(@:.o=.c)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMFILES)

FRC:
