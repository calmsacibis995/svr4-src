#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbps:ps.mk	1.2.3.1"


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

#       Makefile for ps in BSD Compatiblity Package

ROOT=
INC = $(ROOT)/usr/include
LDFLAGS = -s -lw $(SHLIBS)
CFLAGS = -O -I$(INC)
INSDIR = $(ROOT)/usr/ucb

all:	ps

ps:	ps.c
	$(CC) $(CFLAGS) -o ps ps.c $(LDFLAGS) 

install: ps
	install -f $(INSDIR) -m 4755 -u root -g sys ps

clean:
	-rm -f ps.o

clobber: clean
	rm -f ps
