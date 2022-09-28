#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/in.named/tools/nslookup/nslookup.mk	1.13.3.2"

#
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# 		PROPRIETARY NOTICE (Combined)
# 
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
# 
# 
# 
# 		Copyright Notice 
# 
# Notice of copyright on this source code product does not indicate 
# publication.
# 
# 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
# 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
# 	          All rights reserved.
#  
#

DASHO=		-O
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP 
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
USRSBIN=	$(ROOT)/usr/sbin
USRLIB=		$(ROOT)/usr/lib
INSTALL=	install

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)
LIBS=		-lresolv -lsocket -lnsl $(SHLIBS)

OBJS=		main.o getinfo.o debug.o send.o skip.o list.o subr.o commands.o 

all:		nslookup

nslookup:	$(OBJS)
		$(CC) -o nslookup $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)


install:	all
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin nslookup
		$(INSTALL) -f $(USRLIB) -m 0444 -u bin -g bin nslookup.help

clean:
		rm -f a.out core commands.c $(OBJS)

clobber:	clean
		rm -f nslookup

FRC:

#
# Header dependencies
#

commands.o:	commands.l \
		res.h \
		$(FRC)

debug.o:	debug.c \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		res.h \
		$(FRC)

getinfo.o:	getinfo.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		res.h \
		$(FRC)

list.o:		list.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/ctype.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		res.h \
		$(FRC)

main.o:		main.c \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/sys/param.h \
		$(INC)/netdb.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		res.h \
		$(FRC)

send.o:		send.c \
		$(INC)/sys/types.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		res.h \
		$(FRC)

skip.o:		skip.c \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/arpa/nameser.h \
		$(FRC)

subr.o:		subr.c \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/sys/types.h \
		$(INC)/netdb.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/nameser.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		res.h \
		$(FRC)
