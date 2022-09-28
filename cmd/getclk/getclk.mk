#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)getclk:getclk.mk	1.1"

ROOT =
DIR = $(ROOT)/sbin
INC = $(ROOT)/usr/include
LDFLAGS = -s $(LDLIBS)
CFLAGS = -O -I$(INC)

all : getclk

install: all
	install -f $(DIR) -m 0744 -u root -g sys getclk

getclk:		getclk.o 
	$(CC) $(CFLAGS)  -o getclk  getclk.o   $(LDFLAGS)


getclk.o:	$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/sys/rtc.h 

clean:
	rm -f getclk.o

clobber: clean
	rm -f getclk
