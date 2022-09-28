#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:crontab/Makefile	1.7.2.1"

#
# Makefile for lp/crontab
#


INS	=	install

LINT	=	$(PFX)lint

CRONTAB	=	$(ROOT)/var/spool/cron/crontabs

OWNER	=	root
GROUP	=	root
MODE	=	444

SRC	=	lp

all:

install:
	$(INS) -m $(MODE) -u $(OWNER) -g $(GROUP) -f $(CRONTAB) $(SRC)

clobber:

clean:

strip:
