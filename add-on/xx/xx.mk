#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.

#ident	"@(#)xx:xx.mk	1.2.1.1"

ADDON =		xx
COMPONENTS =	io ID cmd sys
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
	rm -rf $(ROOT)/usr/src/pkg/xx
	mkdir $(ROOT)/usr/src/pkg/xx
	cd pkg; find . -type f -print | cpio -pdl $(ROOT)/usr/src/pkg/xx

clean:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "FRC=$(FRC)" clean); \
	done

clobber: clean
	rm -rf pkg

FRC:
