#ident	"@(#)Makefile	1.2	92/02/17	JPB"

#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:ID/Makefile	1.3"

##DRIVERS =	scsi sd01 st01 sc01 sw01
DRIVERS =	scsi sd01 st01 sc01 sw01

all:
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) all); \
	done

install:
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) install); \
	done

clean:
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) clean); \
	done

clobber: clean
	for i in $(DRIVERS) ; \
	do \
		(cd $$i ; $(MAKE) clobber); \
	done
