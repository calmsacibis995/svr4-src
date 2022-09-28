#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lnttys:lnttys.mk	1.3.1.1"

ROOT =
INS = install

all:	lnttys.sh lnsxts.sh lnxts.sh
	cp lnttys.sh  lnttys
	cp lnsxts.sh  lnsxts
	cp lnxts.sh  lnxts

install:	all
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g root lnttys
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g root lnsxts
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g root lnxts

clean:

clobber:	clean
	-rm -f lnttys lnsxts lnxts
