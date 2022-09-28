#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fsba:fsba.mk	1.5.1.1"

DIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
SYMLINK = :
LDFLAGS = -s
CFLAGS = -O -I$(INC)
INS = install

all: fsba

fsba:	$(INC)/stdio.h \
	$(INC)/sys/param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/fs/s5ino.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/fs/s5dir.h \
	$(INC)/fcntl.h \
	fsba.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o fsba fsba.o $(SHLIBS)

install: all
	-rm -f $(ROOT)/etc/fsba
	$(INS) -f $(DIR) -m 0555 -u bin -g bin fsba
	-$(SYMLINK) /usr/sbin/fsba $(ROOT)/etc/fsba

clean:
	rm -f *.o

clobber: clean
	rm -f fsba
