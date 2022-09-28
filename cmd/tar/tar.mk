#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)tar:tar.mk	1.4.3.1"

DIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
LDFLAGS = -s
CFLAGS = -O -I$(INC)
INS = install
SYMLINK = :

all: tar

tar: tar.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(SHLIBS) -lcmd

install: all
	-rm -rf $(ROOT)/etc/tar
	$(INS) -f $(DIR) -m 0555 -u bin -g bin tar
	-$(SYMLINK) /usr/sbin/tar $(ROOT)/etc/tar
	-mkdir ./tmp
	-ln tar.dfl ./tmp/tar
	$(INS) -f $(ROOT)/etc/default -m 0444 -u root -g sys ./tmp/tar
	-rm -rf ./tmp

clean:
	rm -f *.o

clobber: clean
	rm -f tar
