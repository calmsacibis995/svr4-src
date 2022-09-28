#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:port_quick/Makefile	1.1.2.1"

INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -Di386 -DSVR40
INS = install
STRIP = strip

all:	layers

layers:
	$(CC) $(CFLAGS) -c layers.c
	$(CC) $(CFLAGS) -o layers layers.o $(LDLIBS) -los

