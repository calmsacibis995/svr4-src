#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.bin/ftp/ftp.mk	1.17.3.1"

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
LIBS=		$(LDLIBS) $(LIB_OPT) -lsocket -lnsl

OBJS=		cmds.o cmdtab.o ftp.o getpass.o glob.o main.o pclose.o \
		ruserpass.o domacro.o

all:		ftp

ftp:		$(OBJS)
		$(CC) -o ftp  $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:	all
		$(INSTALL) -f $(USRBIN) -m 0555 -u bin -g bin ftp

clean:
		rm -f $(OBJS) core a.out

clobber:	clean
		rm -f ftp

FRC:

#
# Header dependencies
#

cmds.o:		cmds.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/arpa/ftp.h \
		$(INC)/arpa/inet.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		$(INC)/ctype.h \
		$(INC)/sys/wait.h \
		$(FRC)

cmdtab.o:	cmdtab.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(FRC)

cmds.o:		cmds.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/arpa/ftp.h \
		$(INC)/arpa/inet.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		$(INC)/ctype.h \
		$(INC)/sys/wait.h \
		$(FRC)

cmdtab.o:	cmdtab.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(FRC)

domacro.o:	domacro.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(FRC)

ftp.o:		ftp.c \
		ftp_var.h \
		$(INC)/setjmp.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/sys/param.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/ftp.h \
		$(INC)/arpa/telnet.h \
		$(INC)/arpa/inet.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		$(INC)/fcntl.h \
		$(INC)/pwd.h \
		$(INC)/varargs.h \
		$(INC)/ctype.h \
		$(FRC)

getpass.o:	getpass.c \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/sgtty.h \
		$(FRC)

glob.o:		glob.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stat.h \
		$(INC)/dirent.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(FRC)

gtdtablesize.o:		gtdtablesize.c \
		$(INC)/sys/resource.h \
		$(FRC)

main.o:		main.c \
		ftp_var.h \
		$(INC)/sys/param.h \
		$(INC)/setjmp.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/arpa/ftp.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/pwd.h \
		$(INC)/varargs.h \
		$(FRC)

pclose.o:	pclose.c \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/sys/param.h \
		$(INC)/sys/wait.h \
		$(FRC)

ruserpass.o:	ruserpass.c \
		$(INC)/stdio.h \
		$(INC)/utmp.h \
		$(INC)/ctype.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/errno.h \
		$(FRC)
