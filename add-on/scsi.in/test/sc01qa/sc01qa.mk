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

#ident	"@(#)scsi.in:test/sc01qa/sc01qa.mk	1.3"

ROOT =
INCL = -I../.. -I../../io/sc01 
INC = $(ROOT)/usr/include
CFLAGS	= -c -O -I$(INC) $(INCL)
LDFLAGS	= -s

all:		sc01qa

sc01qa:		sc01qa.o
	$(CC) $(LDFLAGS) -o sc01qa sc01qa.o

sc01qa.o:	sc01qa.c
	$(CC) $(CFLAGS) sc01qa.c

clean:
	rm -f *.o

clobber:
	rm -f *.o
	rm -f sc01qa
