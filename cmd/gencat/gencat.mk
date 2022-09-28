#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)gencat:gencat.mk	1.3.2.1"

INC=$(ROOT)/usr/include
CFLAGS=-O -I$(INC)
LDFLAGS=-s

BIN=$(ROOT)/usr/bin

OBJS=	gencat.o msg_conv.o cat_misc.o cat_build.o cat_mmp_dump.o


gencat:	$(OBJS)
	$(CC) $(LDFLAGS) -o gencat $(CFLAGS) $(OBJS) $(SHLIBS)

install : gencat
	install -f $(BIN) -m 00555 -u bin -g bin gencat

clean:
	rm -f *.o gencat
	
clobber:
	rm -f *.o gencat gencat
	
.c.o :
	$(CC) -c $(CFLAGS) $<
