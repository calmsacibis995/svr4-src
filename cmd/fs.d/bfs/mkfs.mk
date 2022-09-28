#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bfs.cmds:bfs.cmds/mkfs.mk	1.7.4.1"

ROOT =
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
DASHO = -O
CFLAGS = $(DASHO) -I$(INC) -I$(INCSYS)
LDFLAGS = -s
STRIP = strip
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
FRC =

FILES =\
	mkfs.o

all: mkfs

mkfs: $(FILES)
	$(CC) $(LDFLAGS) -o mkfs $(FILES) $(ROOTLIBS)

install: mkfs
	$(INS) -f $(ROOT)/etc/fs/bfs -m 0555 -u bin -g bin mkfs
	$(INS) -f $(ROOT)/usr/lib/fs/bfs -m 0555 -u bin -g bin mkfs

clean:
	rm -f *.o

clobber: clean
	rm -f mkfs *.o
#
# Header dependencies
#

mkfs.o: mkfs.c \
	$(INC)/stdio.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/vnode.h \
	$(INCSYS)/sys/fs/bfs.h \
	$(INCSYS)/sys/vtoc.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/fcntl.h \
	$(FRC)

