#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)whodo:whodo.mk	1.9.6.1"

ROOT =
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/sbin
SYMLINK = :
CFLAGS = -O
LDFLAGS = -s
INS=install

all:	whodo

whodo:	\
	$(INC)/stdio.h \
	$(INC)/stdlib.h \
	$(INC)/fcntl.h \
	$(INC)/time.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/utmp.h \
	$(INC)/sys/utsname.h \
	$(INC)/sys/stat.h \
	$(INC)/dirent.h \
	$(INC)/sys/procfs.h \
	$(INC)/sys/proc.h \
	whodo.c
	$(CC) -I$(INC) $(CFLAGS) -o whodo whodo.c $(LDFLAGS) $(SHLIBS)

install:	all
	-rm -f $(ROOT)/etc/whodo
	$(INS) -f $(INSDIR) -m 4555 -u root -g bin whodo
	-$(SYMLINK) /usr/sbin/whodo $(ROOT)/etc/whodo

clean:

clobber:	clean
	rm -f whodo
