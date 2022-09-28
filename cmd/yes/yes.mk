#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)yes:yes.mk	1.1.1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

# Makefile for yes

# to install when not privileged
# set $(CH) in the environment to #

ROOT =
DIR = $(ROOT)/bin

#INSDIR = $(ROOT)/bin
INSDIR = $(ROOT)/usr/bin	# New directory structuring

INC = $(ROOT)/usr/include
INS = "install"

LDFLAGS = -s $(LDLIBS)
CFLAGS = -O -I$(INC)
LDLIBS =

all: yes

yes:	yes.o
	$(CC) $(CFLAGS) -o yes yes.o $(LDFLAGS)

yes.o:

install: all
	rm -f $(DIR)/yes
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin yes

clean:
	rm -f yes.o

clobber: clean
	rm -f yes
