#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)clear:clear.mk	1.5.1.1"
#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.


# Makefile for clear.sh

ROOT =
INSDIR = $(ROOT)/usr/bin
INS= install -f
CFLAGS = -O -I$(INC)
LDFLAGS = -s

all: clear 

clear: clear.sh
	cp clear.sh clear

install: all
	$(INS) $(INSDIR) -m 0555 -u bin -g bin clear 

clean:

clobber: 
	rm -f clear 
