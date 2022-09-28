#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.bin/talk/talk.mk	1.13.3.1"

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
USRBIN=		$(ROOT)/usr/bin
INSTALL=	install

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)
LIBS=		-lcurses -ltermlib -lsocket -lnsl $(SHLIBS)

OBJS=		talk.o get_names.o display.o io.o ctl.o init_disp.o\
	  	msgs.o get_addrs.o ctl_transact.o invite.o look_up.o

all:		talk

talk:		$(OBJS)
		$(CC) -o talk  $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:	all
		$(INSTALL) -f $(USRBIN) -m 0555 -u bin -g bin talk

clean:
		rm -f $(OBJS) core a.out

clobber:	clean
		rm -f talk

FRC:

#
# Header dependencies
#

ctl.o:		ctl.c \
		talk_ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

ctl_transact.o:	ctl_transact.c \
		talk_ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(INC)/sys/time.h \
		$(FRC)

display.o:	display.c \
		talk.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

get_addrs.o:	get_addrs.c \
		talk_ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

get_names.o:	get_names.c \
		talk.h \
		ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

init_disp.o:	init_disp.c \
		talk.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(INC)/signal.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/stropts.h \
		$(FRC)

invite.o:	invite.c \
		talk_ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(INC)/sys/time.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		$(FRC)

io.o:		io.c \
		talk.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/sys/time.h \
		$(INC)/sys/filio.h \
		$(FRC)

look_up.o:	look_up.c \
		talk_ctl.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

msgs.o:		msgs.c \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/sys/time.h \
		talk.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)

talk.o:		talk.c \
		talk.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/curses.h \
		$(INC)/utmp.h \
		$(INC)/errno.h \
		$(FRC)
