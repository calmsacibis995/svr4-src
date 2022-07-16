#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright 1988 Intel Corporation
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied under the terms
#	of a license agreement or nondisclosure
#	agreement with Intel Corporation and may 
#	not be copied nor disclosed except in
#	accordance with the terms of that agreement.
#
#ident	"@(#)mbus:cmd/enet/enet.mk	1.3"

ROOT =

DIR =		$(ROOT)/usr/sbin
RCDIR =		$(ROOT)/etc/init.d
INADIR =	Ina.d

INCRT = 	$(ROOT)/usr/include
CFLAGS = 	-I$(INCRT) -O -s
INS =		install
MAINS = 	enetinfo enetload enetreset
OBJECTS = 	enetinfo.o enetload.o enetreset.o
INAFILE = 	ina
RCFILE = 	ina961
FRC =

all: $(MAINS)

install: all
	$(INS) -f $(DIR) -m 0700 -u root -g sys enetreset
	$(INS) -f $(DIR) -m 0700 -u root -g sys enetload
	$(INS) -f $(DIR) -m 0755 -u root -g sys enetinfo

	cp ina_530.sh $(INAFILE)
	$(INS) -f $(DIR) -m 0755 -u root -g sys $(INAFILE)

	cp ina961_530.sh $(RCFILE)
	$(INS) -f $(RCDIR) -m 0755 -u root -g sys $(RCFILE)

	cd $(INADIR); $(MAKE) -f Ina.mk "FRC=$(FRC)" install

enetinfo: enetinfo.o
	$(CC) $(CFLAGS) enetinfo.o -o enetinfo $(SHLIBS)

enetreset: enetreset.o
	$(CC) $(CFLAGS) enetreset.o -o enetreset $(SHLIBS)

clean:
	rm -f $(OBJECTS) $(RCFILE) $(INAFILE)
	cd $(INADIR); $(MAKE) -f Ina.mk "FRC=$(FRC)" clean

clobber:
	rm -f $(OBJECTS) $(RCFILE) $(INAFILE) $(MAINS)
	cd $(INADIR); $(MAKE) -f Ina.mk "FRC=$(FRC)" clobber

enetinfo.o:	enetinfo.c $(INCRT)/sys/types.h $(INCRT)/sys/fcntl.h\
		$(INCRT)/sys/param.h $(INCRT)/sys/stropts.h\
		$(INCRT)/sys/stream.h

enetreset.o:	enetreset.c $(INCRT)/sys/types.h $(INCRT)/sys/fcntl.h\
		$(INCRT)/sys/param.h $(INCRT)/sys/stropts.h\
		$(INCRT)/sys/stream.h

FRC:
