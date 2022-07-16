#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nametoaddr:resolv/resolv.mk	1.3.3.1"

#	Makefile for resolv.so

ROOT=
DIR=		$(ROOT)/usr/lib
INC=		$(ROOT)/usr/include

PICOPT=		-Kpic
CFLAGS=		$(DASHO) $(MORECPP) -DSYSV $(PICOPT) -I$(INC) -D_RESOLV_ABI
LFLAGS=		-dy -G -ztext
STRIP=		strip
SIZE=		size

MAKEFILE=	resolv.mk
LIBNAME=	resolv.so

RESOLVOBJS=	gthostnamadr.o res_comp.o res_debug.o res_init.o \
		res_mkquery.o res_query.o res_send.o sethostent.o \
		strcasecmp.o
INETOBJS=	bindresvport.o byteorder.o ether_addr.o getnetbyaddr.o \
		getnetbyname.o getnetent.o getproto.o getprotoent.o \
		getprotoname.o getservent.o gtservbyname.o gtservbyport.o \
		inet_addr.o inet_lnaof.o inet_mkaddr.o inet_netof.o \
		inet_network.o rcmd.o rexec.o ruserpass.o
SOCKOBJS=	accept.o bind.o connect.o socket.o socketpair.o \
		shutdown.o getsockopt.o setsockopt.o listen.o \
		receive.o send.o _conn_util.o _utility.o \
		getsocknm.o getpeernm.o setsocknm.o setpeernm.o s_ioctl.o
OBJS=		resolv.o $(RESOLVOBJS) $(INETOBJS) $(SOCKOBJS)


PASSECHO=	\"CC=$(CC)\" \"DASHO=$(DASHO)\" \"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"MAKE=$(MAKE)\" \"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\"

PASSTHRU=	"CC=$(CC)" "DASHO=$(DASHO)" "FRC=$(FRC)" "INC=$(INC)" \
		"MAKE=$(MAKE)" "MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)"

all:		resolv.o
		@for i in libresolv libsocket;\
		do\
			cd $$i;\
			if [ x$(CCSTYPE) = xCOFF ] ; \
			then \
			echo "\n===== $(MAKE) -f $$i.mk all PICOPT= $(PASSECHO)";\
			$(MAKE) -f $$i.mk all PICOPT= $(PASSTHRU);\
			else \
			echo "\n===== $(MAKE) -f $$i.mk all $(PASSECHO)";\
			$(MAKE) -f $$i.mk all $(PASSTHRU);\
			fi ;\
			cd ..;\
		done;\
		wait
		$(CC) -o $(LIBNAME) $(CFLAGS) $(LFLAGS) $(OBJS) -lc
		rm -f $(OBJS)

clean:
		@for i in libresolv libsocket;\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk clean $(PASSECHO)";\
			$(MAKE) -f $$i.mk clean $(PASSTHRU);\
			cd ..;\
		done;\
		wait
		rm -f $(OBJS)

clobber:
		@for i in libresolv libsocket;\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk clobber $(PASSECHO)";\
			$(MAKE) -f $$i.mk clobber $(PASSTHRU);\
			cd ..;\
		done;\
		wait
		rm -f $(OBJS) $(LIBNAME)

install:	all
		install -f $(DIR) resolv.so; \
		ln $(DIR)/resolv.so $(DIR)/libresolv.so
		
size:		all
		$(SIZE) $(LIBNAME)

strip:		all
		$(STRIP) $(LIBNAME)

#
# header dependencies
#
resolv.o:	resolv.c \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/tiuser.h \
		$(INC)/netconfig.h \
		$(INC)/netdir.h \
		$(INC)/string.h \
		$(INC)/fcntl.h \
		$(INC)/sys/param.h \
		$(INC)/sys/errno.h \
		$(INC)/sys/utsname.h \
		$(INC)/net/if.h \
		$(INC)/stropts.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/syslog.h \
		$(FRC)
