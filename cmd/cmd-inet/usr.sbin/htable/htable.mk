#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/htable/htable.mk	1.14.3.1"

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
INSTALL=	install

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)
LIBS=		-lsocket -lnsl $(SHLIBS)
YFLAGS=		-d

OBJS=		htable.o parse.o scan.o

all:		htable


install:	htable
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin htable

htable:		$(OBJS)
		$(CC) -o htable $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

y.tab.h:	parse.y
		$(YACC) $(YFLAGS) parse.y
		rm y.tab.c

clobber:	clean
		rm -f htable
clean:
		rm -f $(OBJS) y.tab.h parse.c scan.c

FRC:

#
# Header dependencies
#

htable.o:	htable.c \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		htable.h \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(INC)/sys/socket.h \
		$(INC)/arpa/inet.h \
		$(FRC)

parse.o:	parse.y \
		htable.h \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(FRC)

scan.o:	scan.l \
		htable.h \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(FRC)

lex.yy.o:	lex.yy.c \
		$(FRC)
