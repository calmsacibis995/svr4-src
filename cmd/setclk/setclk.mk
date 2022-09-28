#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)setclk:setclk.mk	1.3.7.1"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -Uu3b -Uvax -Updp11 -Uu3b15 -Uu3b2 -Di386
SYMLINK = :
LDFLAGS = -s
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

all:	setclk

install: all
	-rm -f $(ROOT)/etc/setclk
	-rm -f $(ROOT)/usr/sbin/setclk
	$(INS) -f $(ROOT)/sbin -m 0550 -u bin -g bin setclk
	$(INS) -f $(ROOT)/usr/sbin -m 0550 -u bin -g bin setclk
	-$(SYMLINK) /sbin/setclk $(ROOT)/etc/setclk

setclk:
	$(CC) $(CFLAGS) $(LDFLAGS) -o setclk setclk.c $(ROOTLIBS)

clean:
	-rm -f setclk.o

clobber: clean
	-rm -f setclk

FRC:

#
# Header dependencies
#

setclk: setclk.c \
	$(INC)/stdio.h \
	$(INC)/sys/sysi86.h \
	$(INC)/sys/uadmin.h \
	$(INC)/sys/rtc.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/time.h \
	$(FRC)
