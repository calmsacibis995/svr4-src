#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbsendmail:ucblib/Makefile	1.5.1.1"


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

ROOT = 
INC = $(ROOT)/.usr/include
INS = install
CFLAGS = -I../include -I$(INC)
LDFLAGS = -s $(SHLIBS)

all:	binrmail binmail

binmail:	mail.o
	$(CC) mail.o -o binmail	 -dy -lnsl -lsocket $(LDFLAGS)

binrmail:	rmail.o
	$(CC) rmail.o -o binrmail $(LDFLAGS)

install:	binmail binrmail
	$(INS) -f $(ROOT)/usr/ucblib -m 4551 -u root binmail 
	$(INS) -f $(ROOT)/usr/ucblib -m 4551 -u root binrmail 

clean:	
	rm -f mail.o rmail.o
clobber:	clean
	rm -f binmail binrmail
