#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)swap:swap.mk	1.12.1.1"

ROOT =
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -I$(INCSYS)
SYMLINK = :
LDFLAGS = -s
INS = install

all:	swap

install: all
	-rm -f $(ROOT)/etc/swap
	$(INS) -f $(ROOT)/usr/sbin -m 2755 -u bin -g sys swap
	-$(SYMLINK) /usr/sbin/swap $(ROOT)/etc/swap

swap:
	$(CC) $(CFLAGS) $(LDFLAGS) -o swap swap.c -lelf $(SHLIBS)

clean:
	-rm -f swap.o

clobber: clean
	-rm -f swap

FRC:

#
# Header dependencies
#

swap:	swap.c \
	$(INC)/stdio.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/param.h \
	$(INC)/dirent.h \
	$(INCSYS)/sys/swap.h \
	$(INCSYS)/sys/immu.h \
	$(INCSYS)/sys/sysmacros.h \
	$(INCSYS)/sys/mkdev.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/uadmin.h \
	$(FRC)
