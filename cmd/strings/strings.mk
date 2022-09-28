#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)strings:strings.mk	1.5.1.1"
#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.


# Makefile for strings

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS = install

LIBELF = -lelf
LDFLAGS = -s $(LIBELF) $(SHLIBS)
CFLAGS = -O -I$(INC) -Di386

all: strings

strings:	strings.o
	$(CC) $(CFLAGS) -o strings strings.o $(LDFLAGS)

strings.o:

install: all
	$(INS) -f $(DIR) -m 0555 -u bin -g bin strings

clean:
	rm -f strings.o

clobber: clean
	rm -f strings
