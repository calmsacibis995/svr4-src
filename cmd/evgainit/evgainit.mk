#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)evgainit:evgainit.mk	1.1"

#
# 	evgainit.mk:
# 	makefile for the evgainit command
#

INS = install
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
OPT = -O
CFLAGS = -I$(INCSYS) -I$(INC) ${OPT} -DEVGA -D_LTYPES
LDFLAGS = -s 
LDLIBS =

all:	evgainit

install:	all
		rm -f $(ROOT)/sbin/evgainit
		$(INS) -f $(ROOT)/sbin -m 0554 -u 0 -g 3 ./evgainit

clean:
	rm -f *.o

clobber: clean
	rm -f evgainit

evgainit:	evgainit.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/kd.h \
	$(FRC)
	$(CC) $(CFLAGS) -o evgainit evgainit.c $(LDFLAGS) $(ROOTLIBS)
