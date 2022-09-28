#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)env:env.mk	1.4.1.1"
#	env make file

ROOT =
OL = $(ROOT)/
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
INSDIR = $(OL)usr/bin
IFLAG = -n
CFLAGS = -O $(FFLAG) -I$(INC)
LDFLAGS = -s $(IFLAG)
SOURCE = env.c
MAKE = make

env:	$(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o env env.c -lm $(SHLIBS)

install: env
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin env

clean:
	rm -f env.o

clobber:	clean
	  rm -f env
