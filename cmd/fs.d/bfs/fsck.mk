#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bfs.cmds:bfs.cmds/fsck.mk	1.11.5.1"


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
	fsck.o

all: fsck

fsck: $(FILES)
	$(CC) $(LDFLAGS) -o fsck $(FILES) $(ROOTLIBS)

clean:
	rm -f *.o

install: fsck
	$(INS) -f $(ROOT)/etc/fs/bfs -m 0555 -u bin -g bin fsck
	$(INS) -f $(ROOT)/usr/lib/fs/bfs -m 0555 -u bin -g bin fsck


clobber: clean
	rm -f fsck

#
# Header dependencies
#

fsck.o: fsck.c \
	$(INC)/stdio.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/fcntl.h \
	$(INCSYS)/sys/vfs.h \
	$(INCSYS)/sys/vnode.h \
	$(INCSYS)/sys/fs/bfs.h \
	$(INCSYS)/sys/fs/bfs_compact.h \
	$(INCSYS)/sys/stat.h \
	$(FRC)
