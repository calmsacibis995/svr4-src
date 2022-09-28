#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-streams:kmacct/kmacct.mk	1.6.1.3"

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
USERBIN = $(ROOT)/usr/sbin
CFLAGS = -O -s -I$(INCSYS) -I$(INC) -Di386
FRC =

PRODUCTS = kmacntl kmapr kmamkdb kmaprint

.c:
	$(CC) $(CFLAGS) -o $* $*.c $(SHLIBS)

all: $(PRODUCTS)

install: all
	for n in $(PRODUCTS) ; do \
		$(INS) -f $(USERBIN) -m 0100 -u root -g sys $$n ; \
	done
	
clean:
	

clobber: 
	-rm -f kmacntl kmapr kmaprint

FRC:

# 
# Header Dependencies
#

kmacntl.o:	kmacntl.c \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/kmacct.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(FRC)

kmapr.o:	kmapr.c \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/kmacct.h \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/fcntl.h \
		$(INC)/errno.h \
		$(FRC)

kmaprint.o:	kmaprint.c \
		$(INC)/stdio.h \
		$(FRC)
