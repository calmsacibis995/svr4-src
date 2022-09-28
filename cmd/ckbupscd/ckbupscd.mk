#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ckbupscd:ckbupscd.mk	1.2.4.1"
#
#		Copyright 1984 AT&T
#

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
SYMLINK = :
INS = install
FRC =

all:	 ckbupscd

install: all
	-rm -f $(ROOT)/etc/ckbupscd
	$(INS) -f $(ROOT)/usr/sbin -m 0755 -u root -g sys ckbupscd
	$(SYMLINK) /usr/sbin/ckbupscd $(ROOT)/etc/ckbupscd

ckbupscd:
	$(CC) $(CFLAGS) $(LDFLAGS) -o ckbupscd ckbupscd.c $(SHLIBS)

clean:
	-rm -f ckbupscd.o

clobber: clean
	-rm -f ckbupscd

FRC:

#
# Header dependencies
#

ckbupscd: ckbupscd.c \
	$(INC)/time.h \
	$(INC)/stdio.h \
	$(INC)/sys/types.h \
	$(FRC)
