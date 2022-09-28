#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xinstall:xinstall.mk	1.1.1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

INS=install
FILES = xinstall fixperm custom help inst

all: $(FILES)

install: all
	-mkdir $(ROOT)/etc/perms
	$(INS) -f $(ROOT)/etc/perms inst
	-mkdir $(ROOT)/usr/lib/custom
	$(INS) -f $(ROOT)/sbin xinstall
	$(INS) -f $(ROOT)/sbin fixperm
	$(INS) -f $(ROOT)/sbin custom
	$(INS) -f $(ROOT)/usr/lib/custom help

clean clobber:
	rm -f fixperm.o fixperm custom xinstall help inst

fixperm: fixperm.o
	$(CC) -O -s -o fixperm fixperm.o $(LDLIBS)

xinstall: xinstall.sh
	cp xinstall.sh xinstall

custom: custom.sh
	cp custom.sh custom

help: help.src
	cp help.src help

inst:
	cp inst.perms inst
