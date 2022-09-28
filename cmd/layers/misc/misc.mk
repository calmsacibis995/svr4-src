#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)attwin:cmd/layers/misc/makefile	1.6.2.1"
#
#		Copyright 1985 AT&T
#

CC = cc
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
INS = install
STRIP = strip

all:	ismpx jterm jwin

ismpx:	ismpx.c
ismpx:	$(INC)/stdio.h
ismpx:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o ismpx ismpx.c $(SHLIBS)

jterm:	jterm.c
jterm:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o jterm jterm.c $(SHLIBS)

jwin:	jwin.c
jwin:	$(INC)/sys/jioctl.h
	$(CC) $(CFLAGS) -o jwin jwin.c $(SHLIBS)

install:	all
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 ismpx
	$(STRIP) $(ROOT)/usr/bin/ismpx
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 jterm
	$(STRIP) $(ROOT)/usr/bin/jterm
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 jwin
	$(STRIP) $(ROOT)/usr/bin/jwin

clean:

clobber:
	rm -f ismpx jterm jwin
