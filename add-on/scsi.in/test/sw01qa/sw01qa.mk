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

#ident	"@(#)scsi.in:test/sw01qa/sw01qa.mk	1.3"

ROOT =
INCL = -I../.. -I../../io/sw01 
INC = $(ROOT)/usr/include
CFLAGS	= -c -O -I$(INC) $(INCL)
LDFLAGS	= -s

all:		sw01qa

sw01qa:		sw01qa.o
	$(CC) $(LDFLAGS) -o sw01qa sw01qa.o

sw01qa.o:	sw01qa.c
	$(CC) $(CFLAGS) sw01qa.c

clean:
	rm -f *.o

clobber:
	rm -f *.o
	rm -f sw01qa
