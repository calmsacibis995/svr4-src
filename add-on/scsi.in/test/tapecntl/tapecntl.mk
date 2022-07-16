#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1989 TOSHIBA Corporation
#	  All Rights Reserved

#	Copyright (c) 1989 SORD Computer Corporation
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	TOSHIBA Corporation and SORD Computer Corporation
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:test/tapecntl/tapecntl.mk	1.3.1.1"

ROOT =
INCL = -I../.. 
INC = $(ROOT)/usr/include
CFLAGS	= -c -O -I$(INC) $(INCL) -D_SYSTEMENV=$(SYSTEMENV)
LDFLAGS	= -s

all:		tapecntl

tapecntl:		tapecntl.o
	$(CC) $(LDFLAGS) -o tapecntl tapecntl.o

tapecntl.o:	tapecntl.c 
	$(CC) $(CFLAGS) tapecntl.c

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/cmd ] || mkdir ../../pkg/cmd
	[ -d ../../pkg/cmd/scsi ] || mkdir ../../pkg/cmd/scsi
	cpset tapecntl ../../pkg/cmd/scsi
	cpset tapecntl $(ROOT)/sbin

clean:
	rm -f *.o

clobber: clean
	rm -f tapecntl
