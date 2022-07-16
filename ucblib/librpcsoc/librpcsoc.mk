#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucblibrpcsoc:librpcsoc.mk	1.1.1.2"

#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#
# Makefile for librpcsoc.a
#
LINT	= lint
LIB	= $(ROOT)/usr/ucblib
LIBNAME	= librpcsoc.a
AR	= ar
LORDER	= lorder
TSORT	= $(PFX)tsort
STRIP	= strip
SIZE	= size
INS	= install
INC	= $(ROOT)/usr/include 
INCFLAGS = -I$(INC) -DPORTMAP
CPPFLAGS = -O -Kpic $(INCFLAGS)
CFLAGS	= $(CPPFLAGS) 

OBJS =	clnt_udp.o clnt_tcp.o svc_udp.o svc_tcp.o rtime.o


SRCS = $(OBJS:.o=.c)

$(LIBNAME): $(OBJS)
	-rm -f $(LIBNAME);
	$(AR) cr $(LIBNAME) `$(LORDER) $(OBJS) | $(TSORT)`

lib: $(LIBNAME)

 
install: lib
	$(INS) -m 644 -u bin -g bin -f $(LIB) $(LIBNAME)
lint:
	$(LINT) $(INCFLAGS) $(SRCS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(LIBNAME)

strip:	all
	$(STRIP) $(OBJS)

size:	all
	$(SIZE) $(OBJS)
