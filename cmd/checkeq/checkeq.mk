#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)checkeq:checkeq.mk	1.1.2.1"


# Makefile for checkeq

# to install when not privileged
# set $(CH) in the environment to #

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INS= install -f

LDFLAGS = -s
CFLAGS = 
LDLIBS =

all: checkeq

checkeq:	checkeq.o
	$(CC) $(CFLAGS) -o checkeq checkeq.o $(LDFLAGS)

checkeq.o:

install: all
	$(INS) $(DIR) -m 0555 -u bin -g bin checkeq

clean:
	rm -f checkeq.o

clobber: clean
	rm -f checkeq
