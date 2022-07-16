#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#       Portions Copyright(c) 1988, Sun Microsystems, Inc.
#       All Rights Reserved

#ident	"@(#)ucbprt:prt.mk	1.3.1.1"

#	Makefile for prt.

HDR = hdr

LIBS = lib/libcom.a 

ROOT =

BIN = $(ROOT)/usr/ucb

CFLAGS = -O

LDFLAGS = -s $(SHLIBS)

CMDS = prt	

all:	$(LIBS) $(CMDS)

prt:	prt.o	$(LIBS)
	$(CC) $(LDFLAGS) prt.o $(LIBS) -o prt

prt.o:	prt.c
	$(CC) -c $(CFLAGS) prt.c


$(LIBS):
	cd lib; $(MAKE) -f lib.mk

install:	all
	install -f $(BIN) -u bin -g bin -m 0555 $(CMDS)

clean:
	-rm -f *.o lib/*.o

clobber:	clean
	-rm -f $(CMDS) $(LIBS)
