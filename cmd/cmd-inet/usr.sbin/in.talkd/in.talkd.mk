#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/in.talkd/in.talkd.mk	1.13.3.1"

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
LIBS=		$(LIB_OPT) -lsocket -lnsl $(SHLIBS)

OBJS=		in.talkd.o announce.o process.o table.o print.o

all:		in.talkd

in.talkd:	$(OBJS)
		$(CC) -o in.talkd  $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:	all
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin in.talkd

clean:
		rm -f $(OBJS)

clobber:	clean
		rm -f in.talkd

FRC:

#
# Header dependencies
#

announce.o:	announce.c \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/sys/stat.h \
		$(INC)/sgtty.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/time.h \
		$(INC)/stdio.h \
		$(INC)/sys/wait.h \
		$(INC)/errno.h \
		$(FRC)

in.talkd.o:	in.talkd.c \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/time.h \
		$(FRC)

print.o:	print.c \
		$(INC)/stdio.h \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(FRC)

process.o:	process.c \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/utmp.h \
		$(FRC)

table.o:	table.c \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/sys/time.h \
		$(FRC)
