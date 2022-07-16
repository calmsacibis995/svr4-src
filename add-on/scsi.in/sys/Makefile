#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:sys/Makefile	1.3"

HEADERS=scsi.h sdi.h sdi_edt.h cdrom.h worm.h

all:	$(HEADERS)

install:	all
	[ -d ../pkg ] || mkdir ../pkg
	[ -d ../pkg/sys ] || mkdir ../pkg/sys
	cp $(HEADERS)  ../pkg/sys

clean:

clobber:
