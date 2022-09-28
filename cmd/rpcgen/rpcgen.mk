#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rpcgen:rpcgen.mk	1.10.4.1"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
# 
#	@(#)Makefile 1.14 89/03/30 (C) 1987 SMI
#
# Makefile for rpc protocol compiler
# Copyright (C) 1987, Sun Microsystems, Inc.
#

DESTDIR = $(ROOT)/usr/bin

ROOT =

INC = $(ROOT)/usr/include

CFLAGS = -O -I$(INC)

LDFLAGS = -s

STRIP = strip

SIZE = size

DIR = $(ROOT)/usr/bin

INS = install

MAINS = rpcgen

OBJECTS = rpc_clntout.o rpc_cout.o rpc_hout.o rpc_main.o rpc_parse.o \
  	  rpc_scan.o rpc_svcout.o rpc_tblout.o rpc_util.o

ALL:	$(MAINS)

$(MAINS): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rpcgen $(OBJECTS) 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	@if [ ! -d $(DIR) ] ; \
	then \
		mkdir $(DIR); \
	fi
	$(INS) -f $(DIR) -m 0555 -u bin -g bin $(MAINS)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
