#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)nametoaddr:tcpip/tcpip.mk	1.2.4.1"

#	Makefile for tcpip.so

ROOT =

DIR = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = -s 

CFLAGS = -O -DPIC -I$(INC) -D_NSL_RPC_ABI 

STRIP = strip

SIZE = size

MAKEFILE = tcpip.mk

MAINS = tcpip.so

OBJECTS =  tcpip.o file_db.o

SOURCES =  tcpip.c file_db.c

ALL:		$(MAINS)

tcpip.so:	tcpip.o  file_db.o
	$(CC) $(CFLAGS) -dy -G -ztext -o tcpip.so tcpip.o file_db.o $(LDFLAGS)


tcpip.o:   $(INC)/stdio.h $(INC)/ctype.h $(INC)/sys/types.h \
	   $(INC)/sys/socket.h $(INC)/netinet/in.h $(INC)/netdb.h  \
	   $(INC)/tiuser.h $(INC)/netconfig.h $(INC)/netdir.h \
	   $(INC)/string.h $(INC)/sys/param.h $(INC)/sys/utsname.h
	$(CC) $(CFLAGS) -c tcpip.c $(LDFLAGS) -Kpic

file_db.o: $(INC)/stdio.h $(INC)/ctype.h $(INC)/string.h  \
	   $(INC)/sys/types.h $(INC)/sys/socket.h $(INC)/netdb.h  \
	   $(INC)/netinet/in.h
	$(CC) $(CFLAGS) -c file_db.c $(LDFLAGS) -Kpic

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	install -f $(DIR) tcpip.so

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
