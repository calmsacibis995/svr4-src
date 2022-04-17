#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)autopush:autopush.mk	1.10.1.1"
#
# autopush.mk:
# makefile for autopush(1M) command
#

INS = install
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
SYMLINK = :
OPT = -O
ARCH = AT386
BUS = AT386
CFLAGS = -I$(INCSYS) -I$(INC) $(OPT) -D$(ARCH) -D$(BUS)
LDFLAGS = -s
DIR = $(ROOT)/sbin

all:	autopush

install:	all
		-rm -f $(ROOT)/etc/autopush
		-rm -f $(ROOT)/usr/sbin/autopush
		$(INS) -f $(DIR) -m 0555 -u root -g sys autopush
		$(INS) -f $(ROOT)/usr/sbin -m 0555 -u root -g sys autopush
		-$(SYMLINK) /sbin/autopush $(ROOT)/etc/autopush
		-mkdir $(ROOT)/etc/ap
		$(CH)chmod 755 $(ROOT)/etc/ap
		$(CH)chgrp sys $(ROOT)/etc/ap
		$(CH)chown root $(ROOT)/etc/ap
		$(INS) -f $(ROOT)/etc/ap -m 0444 -u 0 -g 3 ./i386/chan.ap

clean:
	-rm -f *.o

clobber: clean
	-rm -f autopush

autopush:	autopush.c \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/sad.h \
		$(INCSYS)/sys/conf.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/errno.h \
		$(INC)/ctype.h
		$(CC) $(CFLAGS) -o autopush autopush.c $(LDFLAGS) $(ROOTLIBS)
