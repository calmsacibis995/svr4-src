#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)mkdir:mkdir.mk	1.11.8.1"
#	mkdir make file
ROOT=
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O
LDFLAGS =  -s -lgen $(PERFLIBS)
INS = install

all:	mkdir

mkdir:
	$(CC) $(CFLAGS) -o mkdir mkdir.c  $(LDFLAGS)

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin mkdir

clean:
	-rm -f mkdir.o

clobber: clean
	rm -f mkdir
