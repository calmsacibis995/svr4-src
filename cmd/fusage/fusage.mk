#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fusage:fusage.mk	1.3.10.1"



ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/sbin
SYMLINK = :
CFLAGS = -O -c -I$(INC) 
LDFLAGS = -s
INS=install

all:	fusage

fusage: fusage.o 
	$(CC) -o fusage $(LDFLAGS) fusage.o -lelf $(SHLIBS)

fusage.o: fusage.c $(INC)/stdio.h $(INC)/sys/types.h $(INC)/sys/stat.h \
		$(INC)/sys/statfs.h $(INC)/sys/mnttab.h $(INC)/sys/utsname.h \
		$(INC)/nserve.h $(INC)/ctype.h $(INC)/sys/param.h \
		$(INC)/sys/fcntl.h $(INC)/sys/nserve.h $(INC)/sys/rf_sys.h \
		$(INC)/sys/vfs.h  $(INC)/errno.h $(INC)/nlist.h


install: fusage
	-rm -f $(ROOT)/usr/bin/fusage
	$(INS) -f $(INSDIR) -m 2555 -u bin -g sys fusage
	-$(SYMLINK) /usr/sbin/fusage $(ROOT)/usr/bin/fusage

clean:
	-rm -f fusage.o

clobber: clean
	rm -f fusage
