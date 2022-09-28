#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)hd:hd.mk	1.1.1.1"

# Makefile for hd

# to install when not privileged
# set $(CH) in the environment to #

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)
CFLAGS = -O -I$(INC) -DM_SYS3
LDLIBS =

all: hd

hd:	hd.o
	$(CC) $(CFLAGS) -o hd hd.o $(LDFLAGS)

hd.o:

install: all
	rm -f $(DIR)/hd
	install -f $(DIR) -m 0775 -u bin -g bin hd

clean:
	rm -f hd.o

clobber: clean
	rm -f hd
