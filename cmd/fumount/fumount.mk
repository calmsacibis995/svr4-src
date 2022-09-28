#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fumount:fumount.mk	1.4.13.1"



#	fumount make file

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/sbin
SYMLINK = :
CFLAGS = -O -c -I$(INC)
LDFLAGS = -s
INS=install

all:	fumount

fumount: fumount.o sndmes.o
	$(CC) -o fumount $(LDFLAGS) fumount.o sndmes.o $(SHLIBS)
	$(CH)chmod 755 fumount

fumount.o: fumount.c \
	$(INC)/stdio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/rf_messg.h \
	$(INC)/sys/idtab.h \
	$(INC)/sys/nserve.h \
	$(INC)/sys/rf_sys.h \
	$(INC)/nserve.h 
	$(CC) $(CFLAGS) fumount.c

sndmes.o: sndmes.c \
	$(INC)/stdio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vnode.h \
	$(INC)/sys/rf_messg.h \
	$(INC)/sys/nserve.h \
	$(INC)/sys/rf_sys.h 
	$(CC) $(CFLAGS) sndmes.c

install: fumount
	-rm -f $(ROOT)/usr/bin/fumount
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin fumount
	-$(SYMLINK) /usr/sbin/fumount $(ROOT)/usr/bin/fumount

clean:
	-rm -f fumount.o sndmes.o

clobber: clean
	rm -f fumount

