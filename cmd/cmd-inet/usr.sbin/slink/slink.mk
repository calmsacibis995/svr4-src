#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/slink/slink.mk	1.10.3.1"

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

#	Copyright 1987, 1988 Lachman Associates, Incorporated (LAI)
#	All Rights Reserved.
#
#	The copyright above and this notice must be preserved in all
#	copies of this source code.  The copyright above does not
#	evidence any actual or intended publication of this source
#	code.
#
#	This is unpublished proprietary trade secret source code of
#	Lachman Associates.  This source code may not be copied,
#	disclosed, distributed, demonstrated or licensed except as
#	expressly authorized by Lachman Associates.
#
#	System V STREAMS TCP was jointly developed by Lachman
#	Associates and Convergent Technologies.
#

DASHO=		-O
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
USRSBIN=	$(ROOT)/usr/sbin
INSTALL=	install
LIBS=		$(LIB_OPT)

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)

OBJS=		main.o parse.o exec.o builtin.o slink.o

all:		slink

install:	slink
		$(INSTALL) -f $(USRSBIN) -m 0500 -u root -g bin slink

slink:		$(OBJS)
		$(CC) -o slink $(CFLAGS) $(LDFLAGS) $(OBJS) $(SHLIBS)

clean:
		rm -f $(OBJS) lex.yy.c slink.c

clobber:	clean
		rm -f slink

FRC:

#
# Header dependencies
#

builtin.o:	builtin.c \
		$(INC)/fcntl.h \
		$(INC)/sys/types.h \
		$(INC)/stropts.h \
		$(INC)/sys/stream.h \
		$(INC)/sys/socket.h \
		$(INC)/net/if.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/sockio.h \
		$(INC)/net/strioc.h \
		$(INC)/sys/dlpi.h \
		defs.h \
		$(FRC)

exec.o:		exec.c \
		$(INC)/varargs.h \
		$(INC)/stdio.h \
		defs.h \
		$(FRC)

main.o:		main.c \
		$(INC)/stdio.h \
		$(INC)/varargs.h \
		$(INC)/signal.h \
		defs.h \
		$(FRC)

parse.o:	parse.c \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/varargs.h \
		$(INC)/sys/types.h \
		defs.h \
		$(FRC)

slink.o:	slink.l \
		defs.h \
		$(FRC)
