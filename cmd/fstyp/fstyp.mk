#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fstyp:fstyp.mk	1.3.5.1"
#
#	Makefile for the fstyp component
#

ROOT =
DIR = $(ROOT)/usr/sbin
DIRSV = $(ROOT)/usr/lib/fs/s5
DIRSV1 = $(ROOT)/etc/fs/s5
DIRBFS = $(ROOT)/usr/lib/fs/bfs
DIRBFS1 = $(ROOT)/etc/fs/bfs
SYMLINK = :
INS = install


INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
LDFLAGS =
CFLAGS = -O -s -I$(INC) -I$(INCSYS)
STRIP = strip

OBJECTS =  S5fstyp.o  

all: fstyp S5fstyp Bfsfstyp

S5fstyp:  S5fstyp.o
	$(CC) $(CFLAGS)  -o S5fstyp  S5fstyp.o  $(LDFLAGS) $(ROOTLIBS)

Bfsfstyp:   Bfsfstyp.c
	$(CC) $(CFLAGS)  -o Bfsfstyp Bfsfstyp.c $(LDFLAGS) $(ROOTLIBS)

S5fstyp.o:	$(INCSYS)/sys/param.h $(INCSYS)/sys/stat.h \
		$(INC)/time.h \
		$(INCSYS)/sys/types.h  \
		$(INCSYS)/sys/vnode.h $(INCSYS)/sys/fs/s5param.h \
		$(INCSYS)/sys/fs/s5ino.h $(INCSYS)/sys/fs/s5inode.h \
		$(INCSYS)/sys/fs/s5dir.h $(INC)/stdio.h \
		$(INC)/setjmp.h $(INCSYS)/sys/fs/s5filsys.h \
		$(INCSYS)/sys/fcntl.h

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f fstyp S5fstyp Bfsfstyp 

install: all dir
	-rm -f $(ROOT)/etc/fstyp
	$(INS) -f $(DIR) -m 0555 -u root -g sys fstyp
	$(INS) -f $(ROOT)/sbin -m 0555 -u root -g sys fstyp
	-$(SYMLINK) /sbin/fstyp $(ROOT)/etc/fstyp
	-mkdir ./tmp
	-ln S5fstyp ./tmp/fstyp
	$(INS) -f $(DIRSV) -m 0555 -u root -g sys ./tmp/fstyp
	$(INS) -f $(ROOT)/etc/fs/s5 -m 0555 -u root -g sys ./tmp/fstyp
	-ln Bfsfstyp ./tmp/fstyp
	$(INS) -f $(DIRBFS) -m 0555 -u root -g sys ./tmp/fstyp
	$(INS) -f $(ROOT)/etc/fs/bfs -m 0555 -u root -g sys ./tmp/fstyp
	-rm -rf ./tmp

dir:
	-if [ ! -d $(DIRSV) ] ;\
	then \
		mkdir -p $(DIRSV) ;\
	fi
	-if [ ! -d $(DIRSV1) ] ;\
	then \
		mkdir -p $(DIRSV1) ;\
	fi
	-if [ ! -d $(DIRBFS) ] ;\
	then \
		mkdir -p $(DIRBFS) ;\
	fi
	-if [ ! -d $(DIRBFS1) ] ;\
	then \
		mkdir -p $(DIRBFS1) ;\
	fi

strip: all
	$(STRIP) S5fstyp Bfsfstyp 
