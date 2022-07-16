#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)libcmd:libcmd.mk	1.2.1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.


#
#	@(#) libcmd.mk 1.2 88/05/03 libcmd:libcmd.mk
#

ROOT =
USRLIB 	= $(ROOT)/usr/lib
INC 	= $(ROOT)/usr/include
CFLAGS	= $(GCFLAGS) -O -I$(INC)

INS 	= install

MAKEFILE= libcmd.mk

LIBRARY = libcmd.a

SRCS	=  magic.c sum.c deflt.c getterm.c
OBJS	=  magic.o sum.o deflt.o getterm.o
LIBOBJS	= \
	$(LIBRARY)(magic.o) \
	$(LIBRARY)(sum.o) \
	$(LIBRARY)(deflt.o) \
	$(LIBRARY)(getterm.o)

all:	$(LIBRARY)

install:	all
	$(INS) -f $(USRLIB) -m 0444 -u bin -g bin $(LIBRARY) 
# local directory, nothing to copy

clean:
	-rm -f $(OBJS)

clobber: clean
	-rm -f $(LIBRARY)

FRC:

$(LIBOBJS):	$(FRC)

.PRECIOUS:	$(LIBRARY)

$(LIBRARY):	$(LIBOBJS)

objs:	$(OBJS)
