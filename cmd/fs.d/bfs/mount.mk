#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bfs.cmds:bfs.cmds/mount.mk	1.8.6.1"


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
	mount.o

all: mount

mount: $(FILES)
	$(CC) $(LDFLAGS) -o mount $(FILES) $(ROOTLIBS)

install: mount
	$(INS) -f $(ROOT)/etc/fs/bfs -m 0555 -u bin -g bin mount
	$(INS) -f $(ROOT)/usr/lib/fs/bfs -m 0555 -u bin -g bin mount

clean:
	rm -f *.o

clobber: clean
	rm -f mount

#
# Header dependencies
#

mount.o: mount.c \
	$(INC)/stdio.h \
	$(INC)/signal.h \
	$(INC)/unistd.h \
	$(INC)/errno.h \
	$(INCSYS)/sys/mnttab.h \
	$(INCSYS)/sys/mount.h \
	$(INCSYS)/sys/types.h \
	$(FRC)
