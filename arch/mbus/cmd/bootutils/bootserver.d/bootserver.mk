#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988, 1989 Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/bootutils/bootserver.d/bootserver.mk	1.3"

INCRT = $(ROOT)/usr/include
CC = cc
CFLAGS = -I$(INCRT) 
TAILIB = -lmb2 -lbps
LDFLAGS = -s -O $(IFLAG) 
SBIN   = $(ROOT)/sbin

SOURCES = bootserver.c config.c logfile.c
OBJECTS = $(SOURCES:.c=.o)
MAIN = bootserver 

all:	$(MAIN) 

install: all
	[ -d $(SBIN) ] || mkdir $(SBIN)
	install -m 750 -u root -g daemon -f $(SBIN) $(MAIN)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(MAIN)

bootserver:	$(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(TAILIB) $(ROOTLIBS)

bootserver.o:	bootserver.c bootserver.h $(FRC)

config.o:	config.c bootserver.h $(FRC)

logfile.o:	logfile.c bootserver.h $(FRC)

FRC:
