#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)wsinit:wsinit.mk	1.3"

#
# 	wsinit.mk:
# 	makefile for the wsinit command
#

INS = install
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
OPT = -O
CFLAGS = -I$(INCSYS) -I$(INC) ${OPT} -D_LTYPES
LDFLAGS = -s 
LDLIBS =

all:	wsinit

install:	all
		rm -f $(ROOT)/sbin/wsinit
		rm -f $(ROOT)/etc/default/workstations
		cp ./wstations.sh workstations
		$(INS) -f $(ROOT)/sbin -m 0554 -u 0 -g 3 ./wsinit
		$(INS) -f $(ROOT)/etc/default -m 0644 -u 0 -g 3 ./workstations

clean:
	rm -f *.o

clobber: clean
	rm -f workstations
	rm -f wsinit

wsinit:	wsinit.c \
	$(INC)/sys/genvid.h \
	$(INC)/sys/types.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/mkdev.h \
	$(INC)/sys/stat.h
	$(CC) $(CFLAGS) -o wsinit wsinit.c $(LDFLAGS) $(ROOTLIBS)
