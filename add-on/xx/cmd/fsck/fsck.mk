#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.

#ident	"@(#)xx:cmd/fsck/fsck.mk	1.2.1.1"
# derived from fsck:fsck.mk 1.5
# fsck.mk
TESTDIR = .
INSDIR = $(ROOT)/etc/fs/XENIX
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INC) -I../..
LDFLAGS = -s
FRC =

all:	fsck

fsck: fsck1.o fsck2.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/fsck fsck1.o fsck2.o

fsck1.o:\
	fsck.h\
	$(INC)/stdio.h\
	$(INC)/ctype.h\
	$(INC)/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5param.h\
	../../sys/fs/xxfilsys.h\
	$(INC)/sys/fs/s5dir.h\
	../../sys/fs/xxfblk.h\
	$(INC)/sys/ino.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/fs/s5inode.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/ustat.h\
	fsck1.c\
	$(FRC)

fsck2.o:\
	$(INC)/sys/sysmacros.h\
	fsck.h\
	$(INC)/stdio.h\
	$(INC)/ctype.h\
	$(INC)/signal.h\
	$(INC)/sys/types.h\
	$(INC)/sys/param.h\
	$(INC)/sys/fs/s5param.h\
	../../sys/fs/xxfilsys.h\
	$(INC)/sys/fs/s5dir.h\
	../../sys/fs/xxfblk.h\
	$(INC)/sys/ino.h\
	$(INC)/sys/inode.h\
	$(INC)/sys/fs/s5inode.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/ustat.h\
	fsck2.c\
	$(FRC)

FRC :

install: fsck
	cp fsck ../../pkg

clean:
	rm -f *.o fsck

clobber : clean
	rm -f ../../pkg/fsck
