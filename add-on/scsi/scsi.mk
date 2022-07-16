#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:scsi.mk	1.3"

ADDON =		scsi
COMPONENTS =	cmd io ID
FRC =

all:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" all); \
	done

install: FRC
	[ -d pkg ] || mkdir pkg
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" install); \
	done

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	[ -d $(ROOT)/usr/src/pkg/scsi.src ] || mkdir $(ROOT)/usr/src/pkg/scsi.src
	cd pkg; (find . -print) | cpio -pdu $(ROOT)/usr/src/pkg/scsi.src

clean:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" clean); \
	done

clobber: clean
	rm -rf pkg
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" clobber); \
	done
FRC:
