#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/ots/ots.mk	1.3"

#	Makefile for the SV-ots commands

DIR =		$(ROOT)/usr/sbin
INC =		$(ROOT)/usr/include
INCRT = 	$(INC)
CFLAGS = 	-I$(INCRT) -O -s
INS =		install

MAIN = 		otsdebug \
		otsreset \
		otsstats
OBJ = 		otsdebug.o \
		otsreset.o \
		otsstats.o

FRC =

all: $(MAIN)

install: all
	$(INS) -f $(DIR) -m 0700 -u root -g sys otsreset
	$(INS) -f $(DIR) -m 0700 -u root -g sys otsdebug
	$(INS) -f $(DIR) -m 0755 -u root -g sys otsstats

otsdebug: otsdebug.o
	$(CC) $(CFLAGS) otsdebug.o -o otsdebug $(SHLIBS)

otsstats: otsstats.o
	$(CC) $(CFLAGS) otsstats.o -o otsstats $(SHLIBS)

otsreset: otsreset.o
	$(CC) $(CFLAGS) otsreset.o -o otsreset $(SHLIBS)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(OBJ) $(MAIN)

FRC:
