#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)conv-cmd:conv-cmd.mk	1.3.1.1"
#
# conv-cmd.mk: makefile for 4.0 conversion commands
#

INC = $(ROOT)/usr/include
OPT = -O
CFLAGS = -I$(INC) $(OPT) 
LDFLAGS = -s
INS = install
CMDDIR = $(ROOT)/usr/sbin

CFILES =\
	ttyconv

SHFILES =

PRODUCTS = ttyconv

all:		$(PRODUCTS)

ttyconv:	ttyconv.o
		$(CC) $(CFLAGS) -o ttyconv ttyconv.o $(LDFLAGS) $(SHLIBS)

install:	all
		$(INS) -f $(CMDDIR) -m 0555 -u root -g root ttyconv

clean:
		-rm -f *.o
	
clobber:	clean
		-rm -f $(PRODUCTS)

FRC:
