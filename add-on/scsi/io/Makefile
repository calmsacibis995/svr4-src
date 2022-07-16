#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:io/Makefile	1.3"

ADDON =		scsi
COMPONENTS =	scsi sd01
FRC =

all:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" all); \
	done

install: FRC
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" install); \
	done

clean:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" clean clobber); \
	done

clobber: clean
	rm -rf pkg

FRC:
