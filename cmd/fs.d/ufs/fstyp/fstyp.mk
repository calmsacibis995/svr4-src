#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/fstyp/fstyp.mk	1.3.3.1"

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/fs/ufs
DIRDIR = $(ROOT)/usr/lib/fs
CFLAGS = -O
LDFLAGS = -s 
INS=install
OBJS=

all:  install clobber

fstyp: fstyp.c $(OBJS)
	$(CC) -I$(INC) $(CFLAGS) $(LDFLAGS) -o fstyp fstyp.c $(OBJS) $(SHLIBS)

install: fstyp
	if [ ! -d $(DIRDIR) ]; \
	then \
		mkdir $(DIRDIR); \
	fi
	if [ ! -d $(INSDIR) ]; \
	then \
		mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin fstyp

clean:
	-rm -f fstyp.o

clobber: clean
	rm -f fstyp
