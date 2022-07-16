#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbtermcap:libtermcap.mk	1.2.1.1"

#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.


#
# makefile for libtermcap.a
#
#

CC=$(PFX)cc
CFLAGS= -O -DM_N -DCM_GT -DCM_B -DCM_D
AR=$(PFX)ar
LORDER=$(PFX)lorder
TSORT=$(PFX)tsort
PROF=
NONPROF=
INC=$(ROOT)/usr/include
INC1=$(ROOT)/usr/ucbinclude
INCSYS=$(ROOT)/usr/include
INCSYS1=$(ROOT)/usr/ucbinclude
DEFLIST=
SDEFLIST=
INS=install
OWN=bin
GRP=bin
LIB=$(ROOT)/usr/ucblib

OBJECTS =  termcap.o tgoto.o tputs.o

SOURCES =  termcap.c tgoto.c tputs.c

ALL:		 $(OBJECTS) libtermcap.a

termcap.o: $(INC)/ctype.h $(INC1)/sys/ioctl.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c termcap.c
 
tgoto.o: 
		$(CC)  $(CFLAGS) -I$(INC1) -c tgoto.c

tputs.o:	$(INC1)/sgtty.h $(INC)/ctype.h
		$(CC) $(CFLAGS) -I$(INC) -c tputs.c

GLOBALINCS = $(INC)/ctype.h \
	$(INC1)/sgtty.h \
	$(INC1)/sys/ioctl.h 

libtermcap.a: 
	$(AR) q libtermcap.a `$(LORDER) *.o | $(TSORT)`

install: ALL
	$(INS) -f $(LIB) -m 644 -u $(OWN) -g $(GRP) libtermcap.a
	rm -f $(LIB)/libtermlib.a
	ln $(LIB)/libtermcap.a $(LIB)/libtermlib.a

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) libtermcap.a
