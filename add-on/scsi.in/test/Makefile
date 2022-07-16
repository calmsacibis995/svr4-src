#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:test/Makefile	1.3"

SYSTEMENV	= 3
DRIVERS =	sc01qa sw01qa tapecntl

all:
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) -f $$i.mk "SYSTEMENV=$(SYSTEMENV)" all); \
	done

install:	all
	[ -d ../pkg ] || mkdir ../pkg
	[ -d ../pkg/cmd ] || mkdir ../pkg/cmd
	for i in $(DRIVERS) ; \
	do \
		(cpset $$i/$$i ../pkg/cmd); \
	done

clean:
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) -f $$i.mk clean); \
	done

clobber: clean
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) -f $$i.mk clobber); \
	done
