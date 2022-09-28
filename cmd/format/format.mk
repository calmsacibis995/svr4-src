#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)format:format.mk	1.4.2.1"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s
SYMLINK = :
INS = install
ARCH = AT386
FRC =

all:	format

install: all
	-rm -f $(ROOT)/etc/format
	$(INS) -f $(ROOT)/usr/sbin format
	$(SYMLINK) $(ROOT)/usr/sbin/format $(ROOT)/etc/format

format:
	$(CC) -D$(ARCH) $(CFLAGS) $(LDFLAGS) -o format format.c $(SHLIBS)

clean:
	-rm -f format.o

clobber: clean
	-rm -f format

FRC:

#
# Header dependencies
#

format: format.c \
	$(INC)/sys/types.h \
	$(INC)/stdio.h \
	$(INC)/sys/param.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/iobuf.h \
	$(INC)/errno.h \
	$(INC)/sys/vtoc.h \
	$(FRC)
