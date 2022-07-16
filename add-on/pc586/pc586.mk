#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pc586:pc586.mk	1.3"

ADDON =		pc586
COMPONENTS =	ID io sys
FRC=

all:
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) "FRC=$(FRC)" all); \
	done
	@echo "\n\t$(ADDON) build completed"

install:	FRC
	[ -d pkg ] || mkdir pkg
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) "FRC=$(FRC)" install); \
	done

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	rm -rf $(ROOT)/usr/src/pkg/$(ADDON)
	mkdir $(ROOT)/usr/src/pkg/$(ADDON)
	cd pkg; find . -type f -print | cpio -pdl $(ROOT)/usr/src/pkg/$(ADDON)

clean:
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) clean); \
	done

clobber:	clean
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) clobber); \
	done
	rm -rf pkg

FRC:
