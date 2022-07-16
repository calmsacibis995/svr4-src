#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#       Portions Copyright(c) 1988, Sun Microsystems, Inc.
#       All Rights Reserved

#ident	"@(#)ucbdiffmk:diffmk.mk	1.2.1.1"


#	diffmk make file

ROOT =

DIR = $(ROOT)/usr/ucb

INS = install

MAKEFILE = diffmk.mk

MAINS = diffmk

OBJECTS =  diffmk

SOURCES =  diffmk.sh

ALL:		$(MAINS)

diffmk:	
	cp diffmk.sh diffmk

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS)


all : ALL

install: ALL
	$(INS) -f $(DIR)  -m 0555 -u bin -g bin $(MAINS)

