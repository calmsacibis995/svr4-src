#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)test:test.mk	1.1"


#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.


# Makefile for test.sh

ROOT =
DIR = $(ROOT)/usr/bin
INS= install -f

# to install when not privileged
# set $(CH) in the environment to #

all: test

test: test.sh
	cp test.sh test

install: all
	$(INS) $(DIR) -m 0775 -u bin -g bin test 

clean:

clobber: clean
	rm -f test
