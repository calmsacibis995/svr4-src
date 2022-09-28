#ident	"@(#)usr.sbin.mk	1.2	92/12/21	JPB"

#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/usr.sbin.mk	1.26.5.1"

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


DASHO=		-O
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
USRSBIN=	$(ROOT)/usr/sbin
INSTALL=	install

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)

LIBS=		-lsocket -lnsl $(SHLIBS)

#	don't include 'arp' and 'ping' in PRODUCTS list because
#	they have to be installed with set[ug]id on
ALL=		arp ping $(PRODUCTS)
PRODUCTS=	gettable ifconfig in.comsat in.fingerd in.rarpd \
		in.rexecd in.rlogind in.rshd in.rwhod in.telnetd in.tftpd \
		in.tnamed inetd route trpt
DIRS=		htable in.ftpd in.named in.routed in.talkd slink in.bootp

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
			$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin $$i;\
		done
		$(INSTALL) -f $(USRSBIN) -m 02555 -u bin -g bin arp
		$(INSTALL) -f $(USRSBIN) -m 04755 -u root -g bin ping
		@for i in $(DIRS);\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk install $(PASSECHO)";\
			$(MAKE) -f $$i.mk install $(PASSTHRU);\
			cd ..;\
		done;\
		wait

#	these targets use default rule
gettable:	gettable.o
		$(CC) -o gettable $(CFLAGS) $(LDFLAGS) gettable.o $(LIBS)
ifconfig:	ifconfig.o
		$(CC) -o ifconfig $(CFLAGS) $(LDFLAGS) ifconfig.o $(LIBS)
in.comsat:	in.comsat.o
		$(CC) -o in.comsat $(CFLAGS) $(LDFLAGS) in.comsat.o $(LIBS)
in.fingerd:	in.fingerd.o
		$(CC) -o in.fingerd $(CFLAGS) $(LDFLAGS) in.fingerd.o $(LIBS)
in.rarpd:	in.rarpd.o
		$(CC) -o in.rarpd $(CFLAGS) $(LDFLAGS) in.rarpd.o $(LIBS)
in.rexecd:	in.rexecd.o
		$(CC) -o in.rexecd $(CFLAGS) $(LDFLAGS) in.rexecd.o $(LIBS)
in.rlogind:	in.rlogind.o
		$(CC) -o in.rlogind $(CFLAGS) $(LDFLAGS) in.rlogind.o $(LIBS)
in.rshd:	in.rshd.o
		$(CC) -o in.rshd $(CFLAGS) $(LDFLAGS) in.rshd.o $(LIBS)
in.telnetd:	in.telnetd.o
		$(CC) -o in.telnetd $(CFLAGS) $(LDFLAGS) in.telnetd.o $(LIBS)
in.tnamed:	in.tnamed.o
		$(CC) -o in.tnamed $(CFLAGS) $(LDFLAGS) in.tnamed.o $(LIBS)
inetd:		inetd.o
		$(CC) -o inetd $(CFLAGS) $(LDFLAGS) inetd.o $(LIBS)
ping:		ping.o
		$(CC) -o ping $(CFLAGS) $(LDFLAGS) ping.o $(LIBS)

#	these targets can't use default rule -- need extra .o's or libs
arp:		arp.o
		$(CC) -o arp $(CFLAGS) $(LDFLAGS) arp.o $(LIBS) -lelf

in.rwhod:	in.rwhod.o
		$(CC) -o in.rwhod $(CFLAGS) $(LDFLAGS) in.rwhod.o $(LIBS) -lelf

in.tftpd:	in.tftpd.o tftpsubs.o
		$(CC) $(CFLAGS) $(LDFLAGS) -o in.tftpd \
			in.tftpd.o tftpsubs.o $(LIBS)

route:		route.o
		$(CC) -o route $(CFLAGS) $(LDFLAGS) route.o $(LIBS) -lelf

tftpsubs.o:	../usr.bin/tftp/tftpsubs.c
		$(CC) -c $(CFLAGS) ../usr.bin/tftp/tftpsubs.c 

trpt:		trpt.o
		$(CC) -o trpt $(CFLAGS) $(LDFLAGS) trpt.o $(LIBS) -lelf

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
		rm -f *.o arp ping $(PRODUCTS) core a.out
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

arp.o:		arp.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/sys/ioctl.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		$(INC)/nlist.h \
		$(INC)/net/if.h \
		$(INC)/net/if_arp.h \
		$(INC)/netinet/if_ether.h \
		$(INC)/sys/sockio.h \
		$(INC)/fcntl.h \
		$(INC)/stropts.h \
		$(FRC)

gettable.o:	gettable.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/netdb.h \
		$(FRC)

ifconfig.o:	ifconfig.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/sockio.h \
		$(INC)/net/if.h \
		$(INC)/netinet/in.h \
		$(INC)/stropts.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(FRC)

in.comsat.o:	in.comsat.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/file.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/sgtty.h \
		$(INC)/utmp.h \
		$(INC)/signal.h \
		$(INC)/errno.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(FRC)

in.fingerd.o:	in.fingerd.c \
		$(INC)/sys/types.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(FRC)

in.rarpd.o:	in.rarpd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/net/if.h \
		$(INC)/net/if_arp.h \
		$(INC)/netinet/in.h \
		$(INC)/netinet/if_ether.h \
		$(INC)/sys/stropts.h \
		$(INC)/sys/dlpi.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/syslog.h \
		$(INC)/dirent.h \
		$(INC)/signal.h \
		$(INC)/netdb.h \
		$(FRC)

in.rexecd.o:	in.rexecd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/sys/filio.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(INC)/signal.h \
		$(INC)/netdb.h \
		$(FRC)

in.rlogind.o:	in.rlogind.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/file.h \
		$(INC)/sys/stream.h \
		$(INC)/sys/stropts.h \
		$(INC)/netinet/in.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(INC)/signal.h \
		$(INC)/sgtty.h \
		$(INC)/fcntl.h \
		$(INC)/stdio.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(INC)/string.h \
		$(INC)/utmp.h \
		$(INC)/utmpx.h \
		$(INC)/sys/ttold.h \
		$(INC)/sys/filio.h \
		$(FRC)

in.rshd.o:	in.rshd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/inet.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/pwd.h \
		$(INC)/signal.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(INC)/sys/filio.h \
		$(FRC)

in.rwhod.o:	in.rwhod.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/file.h \
		$(INC)/sys/stropts.h \
		$(INC)/net/if.h \
		$(INC)/netinet/in.h \
		$(INC)/nlist.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/errno.h \
		$(INC)/utmp.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(INC)/fcntl.h \
		$(INC)/protocols/rwhod.h \
		$(FRC)

in.telnetd.o:	in.telnetd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/file.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/filio.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stropts.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/telnet.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/errno.h \
		$(INC)/sgtty.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(INC)/ctype.h \
		$(INC)/fcntl.h \
		$(INC)/utmp.h \
		$(INC)/utmpx.h \
		$(INC)/sys/ioctl.h \
		$(FRC)

in.tftpd.o:	in.tftpd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/tftp.h \
		$(INC)/dirent.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/setjmp.h \
		$(INC)/syslog.h \
		$(FRC)

in.tnamed.o:	in.tnamed.c \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/signal.h \
		$(INC)/sys/time.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(FRC)

inetd.o:	inetd.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/file.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/time.h \
		$(INC)/sys/resource.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/inet.h \
		$(INC)/errno.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INC)/netdb.h \
		$(INC)/rpc/rpcent.h \
		$(INC)/syslog.h \
		$(INC)/pwd.h \
		$(INC)/fcntl.h \
		$(INC)/tiuser.h \
		$(INC)/netdir.h \
		$(INC)/ctype.h \
		$(INC)/values.h \
		$(INC)/sys/poll.h \
		$(FRC)

ping.o:		ping.c \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/sys/types.h \
		$(INC)/sys/time.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/sys/file.h \
		$(INC)/sys/signal.h \
		$(INC)/net/if.h \
		$(INC)/netinet/in_systm.h \
		$(INC)/netinet/in.h \
		$(INC)/netinet/ip.h \
		$(INC)/netinet/ip_icmp.h \
		$(INC)/netdb.h \
		$(FRC)

route.o:	route.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/sys/ioctl.h \
		$(INC)/signal.h \
		$(INC)/setjmp.h \
		$(INC)/sys/stream.h \
		$(INC)/sys/stropts.h \
		$(INC)/net/route.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/fcntl.h \
		$(INC)/nlist.h \
		$(FRC)

trpt.o:		trpt.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/stream.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/socketvar.h \
		$(INC)/net/if.h \
		$(INC)/netinet/in.h \
		$(INC)/net/route.h \
		$(INC)/netinet/in_pcb.h \
		$(INC)/netinet/in_systm.h \
		$(INC)/netinet/ip.h \
		$(INC)/netinet/ip_var.h \
		$(INC)/netinet/tcp.h \
		$(INC)/netinet/tcp_fsm.h \
		$(INC)/netinet/tcp_seq.h \
		$(INC)/netinet/tcp_timer.h \
		$(INC)/netinet/tcp_var.h \
		$(INC)/netinet/tcpip.h \
		$(INC)/netinet/tcp_debug.h \
		$(INC)/arpa/inet.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/nlist.h \
		$(INC)/fcntl.h \
		$(INC)/sys/stat.h \
		$(FRC)
