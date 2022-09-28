#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)tset:tset.mk	1.1.1.1"
#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.


#
#	@(#) tset.mk 1.1 88/03/29 tset:tset.mk
#

CFLAGS	= $(GCFLAGS) -DM_SYS3 -O
LDFLAGS	= $(GLDFLAGS) -s
LIBS	= $(LDLIBS) -lcurses -lcmd -lgen

DBIN	= $(ROOT)/usr/bin

SRCS	= tset.c
OBJS	= tset.o
EXES	= tset

all:	tset

install:	all
	install -f $(DBIN) -m 0755 -u bin -g bin  tset 

cmp:	all
	cmp tset $(DBIN)/tset

clean:
	rm -f $(OBJS) delays.i
clobber:	clean
	rm -f $(EXES)

FRC:

$(OBJS):	$(FRC)

tset:	$(OBJS)
	${CC} $(LDFLAGS) -o tset tset.o $(LIBS) 

tset.o:	tset.c delays.i

delays.i: delays
	regcmp delays
