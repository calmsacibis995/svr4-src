#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.bin/usr.bin.mk	1.17.3.1"

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
LIBS=		-lsocket -lnsl $(SHLIBS)

#	don't include 'rcp', 'rlogin' and 'rsh' in PRODUCTS list
#	because they have to be installed with set[gu]id on
ALL=		rcp rlogin rsh $(PRODUCTS)
PRODUCTS=	finger rdate ruptime rwho telnet whois
DIRS=		ftp netstat talk tftp

.o:
		$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $< $(LIBS)


PASSECHO=	\"AR=$(AR)\" \"AS=$(AS)\" \"CC=$(CC)\" \"DASHO=$(DASHO)\" \
		\"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"INSTALL=$(INSTALL)\" \"LD=$(LD)\" \"LDFLAGS=$(LDFLAGS)\" \
		\"LEX=$(LEX)\" \"MAKE=$(MAKE)\"  \
		\"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\" \
		\"YACC=$(YACC)\" \"SHLIBS=$(SHLIBS)\" \"SYMLINK=$(SYMLINK)\"

PASSTHRU=	"AR=$(AR)" "AS=$(AS)" "CC=$(CC)" "DASHO=$(DASHO)" \
		"FRC=$(FRC)" "INC=$(INC)" \
		"INSTALL=$(INSTALL)" "LD=$(LD)" "LDFLAGS=$(LDFLAGS)" \
		"LEX=$(LEX)" "MAKE=$(MAKE)" \
		"MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)" \
		"YACC=$(YACC)" "SHLIBS=$(SHLIBS)" "SYMLINK=$(SYMLINK)"


all:		$(ALL)
		@for i in $(DIRS);\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk all $(PASSECHO)";\
			$(MAKE) -f $$i.mk all $(PASSTHRU);\
			cd ..;\
		done;\
		wait

install:	$(ALL)
		@for i in $(PRODUCTS);\
		do\
			$(INSTALL) -f $(USRBIN) -m 0555 -u bin -g bin $$i;\
		done
		$(INSTALL) -f $(USRBIN) -m 04555 -u root -g bin rcp
		$(INSTALL) -f $(USRBIN) -m 04555 -u root -g bin rlogin
		$(INSTALL) -f $(USRBIN) -m 04555 -u root -g bin rsh
		@for i in $(DIRS);\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk install $(PASSECHO)";\
			$(MAKE) -f $$i.mk install $(PASSTHRU);\
			cd ..;\
		done;\
		wait

rcp:		rcp.o
		$(CC) -o rcp $(CFLAGS) $(LDFLAGS) rcp.o $(TMPLIB) $(LIBS)

rlogin:		rlogin.o
		$(CC) -o rlogin $(CFLAGS) $(LDFLAGS) rlogin.o $(TMPLIB) $(LIBS)

rsh:		rsh.o
		$(CC) -o rsh $(CFLAGS) $(LDFLAGS) rsh.o $(TMPLIB) $(LIBS)

finger:		finger.o
		$(CC) -o finger $(CFLAGS) $(LDFLAGS) finger.o $(TMPLIB) $(LIBS)
rdate:		rdate.o
		$(CC) -o rdate $(CFLAGS) $(LDFLAGS) rdate.o $(TMPLIB) $(LIBS)
ruptime:	ruptime.o
		$(CC) -o ruptime $(CFLAGS) $(LDFLAGS) ruptime.o $(TMPLIB) $(LIBS)
rwho:		rwho.o
		$(CC) -o rwho $(CFLAGS) $(LDFLAGS) rwho.o $(TMPLIB) $(LIBS)
telnet:		telnet.o
		$(CC) -o telnet $(CFLAGS) $(LDFLAGS) telnet.o $(TMPLIB) $(LIBS)
whois:		whois.o
		$(CC) -o whois $(CFLAGS) $(LDFLAGS) whois.o $(TMPLIB) $(LIBS)


clean:
		rm -f *.o core a.out
		@for i in $(DIRS);\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk clean $(PASSECHO)";\
			$(MAKE) -f $$i.mk clean $(PASSTHRU);\
			cd ..;\
		done;\
		wait
clobber:
		rm -f *.o rcp rlogin rsh $(PRODUCTS) core a.out
		@for i in $(DIRS);\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk clobber $(PASSECHO)";\
			$(MAKE) -f $$i.mk clobber $(PASSTHRU);\
			cd ..;\
		done;\
		wait

FRC:

#
# Header dependencies
#

finger.o:	finger.c \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/utmp.h \
		$(INC)/sys/signal.h \
		$(INC)/pwd.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(FRC)

rcp.o:		rcp.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/time.h \
		$(INC)/sys/ioctl.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/pwd.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/errno.h \
		$(INC)/dirent.h \
		$(FRC)

rdate.o:	rdate.c \
		$(INC)/signal.h \
		$(INC)/sys/types.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/stdio.h \
		$(FRC)

rlogin.o:	rlogin.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/errno.h \
		$(INC)/sys/file.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/stropts.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/sgtty.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		$(INC)/netdb.h \
		$(INC)/fcntl.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/sockio.h \
		$(FRC)

rsh.o:		rsh.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/file.h \
		$(INC)/sys/filio.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/signal.h \
		$(INC)/pwd.h \
		$(INC)/netdb.h \
		$(FRC)

ruptime.o:	ruptime.c \
		$(INC)/sys/param.h \
		$(INC)/stdio.h \
		$(INC)/dirent.h \
		$(INC)/protocols/rwhod.h \
		$(FRC)

rwho.o:		rwho.c \
		$(INC)/sys/param.h \
		$(INC)/stdio.h \
		$(INC)/dirent.h \
		$(INC)/protocols/rwhod.h \
		$(FRC)

telnet.o:	telnet.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stropts.h \
		$(INC)/sys/filio.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/telnet.h \
		$(INC)/arpa/inet.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/errno.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		$(INC)/netdb.h \
		$(INC)/string.h \
		$(INC)/varargs.h \
		$(FRC)

whois.o:	whois.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/netdb.h \
		$(FRC)
