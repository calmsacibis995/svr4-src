#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:smtp/smtp.mk	1.3.3.1"
USR_LIB=/usr/lib
MAILSURRCMD=$(USR_LIB)/mail/surrcmd

smtp: all

all:
	cd scripts; $(MAKE) -f scripts.mk all
	cd src; $(MAKE) -f src.mk all

install:
	cd scripts; $(MAKE) -f scripts.mk install
	cd src; $(MAKE) -f src.mk install

clean:
	cd scripts; $(MAKE) -f scripts.mk clean
	cd src; $(MAKE) -f src.mk clean

clobber:
	cd scripts; $(MAKE) -f scripts.mk clobber
	cd src; $(MAKE) -f src.mk clobber

strip:
	cd scripts; $(MAKE) -f scripts.mk strip
	cd src; $(MAKE) -f src.mk strip
