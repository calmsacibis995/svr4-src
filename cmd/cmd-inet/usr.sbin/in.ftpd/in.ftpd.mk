#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/in.ftpd/in.ftpd.mk	1.18.4.1"

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

OBJS=		ftpd.o ftpcmd.o getusershell.o glob.o popen.o logwtmp.o vers.o

all:		in.ftpd

in.ftpd:	$(OBJS)
		$(CC) -o in.ftpd  $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:	all
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin in.ftpd

clean:
		rm -f $(OBJS) y.tab.c ftpcmd.c y.tab.h

clobber:	clean
		rm -f in.ftpd

FRC:

#
# Header dependencies
#

ftpcmd.o:	ftpcmd.y \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/ftp.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/ctype.h \
		$(INC)/pwd.h \
		$(INC)/setjmp.h \
		$(INC)/syslog.h \
		$(INC)/arpa/telnet.h \
		$(FRC)

ftpd.o:		ftpd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/sys/file.h \
		$(INC)/sys/wait.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/ftp.h \
		$(INC)/arpa/inet.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/pwd.h \
		$(INC)/setjmp.h \
		$(INC)/netdb.h \
		$(INC)/errno.h \
		$(INC)/syslog.h \
		$(INC)/varargs.h \
		$(INC)/fcntl.h \
		$(INC)/shadow.h \
		$(FRC)

getusershell.o:	getusershell.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/file.h \
		$(INC)/sys/stat.h \
		$(INC)/ctype.h \
		$(INC)/stdio.h \
		$(FRC)

glob.o:		../../usr.bin/ftp/glob.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stat.h \
		$(INC)/dirent.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(FRC)
		$(CC) $(CFLAGS) -c ../../usr.bin/ftp/glob.c

logwtmp.o:	logwtmp.c \
		$(INC)/sys/types.h \
		$(INC)/sys/file.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stat.h \
		$(INC)/utmp.h \
		$(INC)/fcntl.h \
		$(FRC)

popen.o:	popen.c \
		$(INC)/sys/types.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(FRC)

vers.o:		vers.c \
		$(FRC)
