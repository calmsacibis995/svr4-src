#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)logname	1.4.1.6"
#	logname make file

ROOT =
OL = $(ROOT)/
INS = install
INSDIR = $(OL)usr/bin
INC =$(ROOT)/usr/include
CFLAGS = -O $(FFLAG) -I$(INC)
LDFLAGS = -s
SOURCE = logname.c
MAKE = make

logname:	$(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o logname logname.c -lm $(SHLIBS)

install:	logname
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin logname

clean:
	rm -f logname.o

clobber:	clean
	  rm -f logname
