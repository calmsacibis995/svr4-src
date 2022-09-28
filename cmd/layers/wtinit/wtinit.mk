#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)attwin:cmd/layers/wtinit/makefile	1.6.2.1"
#
#		Copyright 1985 AT&T
#

CC = cc
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
INS = install
STRIP = strip

all:	wtinit

wtinit:	wtinit.o proto.o pcheck.o
	$(CC) $(CFLAGS) -o wtinit wtinit.o proto.o pcheck.o $(SHLIBS)

wtinit.o:	wtinit.c
wtinit.o:	$(INC)/fcntl.h
wtinit.o:	$(INC)/termio.h
wtinit.o:	$(INC)/a.out.h
wtinit.o:	$(INC)/stdio.h
wtinit.o:	$(INC)/errno.h
wtinit.o:	$(INC)/sys/jioctl.h
wtinit.o:	proto.h
wtinit.o:	$(INC)/signal.h
wtinit.o:	$(INC)/sys/types.h
wtinit.o:	$(INC)/sys/stat.h
	$(CC) $(CFLAGS) -c wtinit.c

proto.o:	proto.c
proto.o:	$(INC)/signal.h
proto.o:	$(INC)/stdio.h
proto.o:	proto.h
	$(CC) $(CFLAGS) -c proto.c

pcheck.o:	pcheck.c
	$(CC) $(CFLAGS) -c pcheck.c


install:	all
	if [ ! -d $(ROOT)/usr/lib/layersys ] ;\
	then \
		mkdir $(ROOT)/usr/lib/layersys ;\
		$(CH)chmod 755 $(ROOT)/usr/lib/layersys ;\
		$(CH)chgrp bin $(ROOT)/usr/lib/layersys ;\
		$(CH)chown bin $(ROOT)/usr/lib/layersys ;\
	fi 
	$(INS) -f $(ROOT)/usr/lib/layersys -u bin -g bin -m 755 wtinit
	$(STRIP) $(ROOT)/usr/lib/layersys/wtinit
clean:
	rm -f wtinit.o proto.o pcheck.o

clobber: clean
	rm -f wtinit
