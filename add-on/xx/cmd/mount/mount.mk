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

#ident	"@(#)xx:cmd/mount/mount.mk	1.2.1.1"
TESTDIR = .
INSDIR = $(ROOT)/etc/fs/XENIX
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INC) -I../..
LDFLAGS = -s
FRC =

all:	mount

mount:	mount.c\
	$(INC)/sys/signal.h\
	$(INC)/unistd.h\
	$(INC)/sys/errno.h\
	$(INC)/sys/mnttab.h\
	$(INC)/sys/mount.h\
	$(INC)/sys/types.h\
	$(INC)/sys/statvfs.h
	$(CC) $(CFLAGS) -o $(TESTDIR)/mount mount.c $(LDFLAGS)

FRC :

install: mount
	cp mount ../../pkg

clean:
	rm -f mount

clobber : clean
	rm -f ../../pkg/mount
