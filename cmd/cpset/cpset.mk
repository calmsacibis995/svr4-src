#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)cpset:cpset.mk	1.1.3.1"

# Makefile for cpset

# to install when not privileged
# set $(CH) in the environment to #

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS= install -f

LDFLAGS = -s
CFLAGS = 
LDLIBS =

all: cpset

cpset:	cpset.o
	$(CC) $(CFLAGS) -o cpset cpset.o $(LDFLAGS)

cpset.o:

install: all
	$(INS) $(DIR) -m 0555 -u bin -g bin cpset

clean:
	rm -f cpset.o

clobber: clean
	rm -f cpset
