#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fsinfo:fsinfo.mk	1.2.3.1"
#
#		Copyright 1984 AT&T
#

ROOT =
INCSYS = $(ROOT)/usr/include
CFLAGS = -O -I$(INCSYS)
LDFLAGS = -s
INS = install

all:	fsinfo

install: all
	$(INS) -f $(ROOT)/usr/lbin -m 0555 -u bin -g bin fsinfo

fsinfo:
	$(CC) $(CFLAGS) $(LDFLAGS) -o fsinfo fsinfo.c $(NOSHLIBS)

clean:
	-rm -f fsinfo.o

clobber: clean
	-rm -f fsinfo

FRC:

#
# Header dependencies
#

fsinfo: fsinfo.c \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/fs/s5ino.h \
	$(INCSYS)/sys/fs/s5param.h \
	$(INCSYS)/sys/fs/s5filsys.h \
	$(INCSYS)/sys/fs/s5dir.h \
	$(INCSYS)/fcntl.h \
	$(FRC)
