#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bfs.cmds:bfs.cmds/bfs.mk	1.9.3.1"


ROOT =
INC = $(ROOT)/usr/include
DASHO = -O
CFLAGS = $(DASHO) -I$(INC)
STRIP = strip
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"
INS = install
DIR = $(ROOT)/etc/fs/bfs $(ROOT)/usr/lib/fs/bfs
FRC =

CMDS =\
	fsck \
	mount \
	mkfs

all: $(CMDS)

fsck:
	$(MAKE) -f fsck.mk

mount:
	$(MAKE) -f mount.mk

mkfs:
	$(MAKE) -f mkfs.mk

clean:
	rm -f *.o

install: $(CMDS) $(DIR)
	$(MAKE) -f fsck.mk install
	$(MAKE) -f mount.mk install
	$(MAKE) -f mkfs.mk install

$(DIR):
	mkdir $@


clobber: clean
	rm -f $(CMDS)
