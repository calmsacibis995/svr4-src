#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/biod/biod.mk	1.5.2.1"
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
CFLAGS = -I$(INC) -O -Dnetselstrings
LDFLAGS = -s
INS = install
INSDIR = $(ROOT)/usr/lib/nfs

all: biod

install: all
	if [ ! -d $(INSDIR) ] ; \
	then mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin biod

clean:
	-rm -f biod.o

clobber: clean
	-rm -f biod

biod:	biod.o
	$(CC) -o biod $(LDFLAGS) biod.o $(SHLIBS)

biod:	$(INC)/stdio.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/file.h \
	$(INCSYS)/sys/ioctl.h
