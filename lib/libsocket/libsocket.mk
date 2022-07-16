#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libsocket:libsocket.mk	1.16.5.1"

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

ROOT=
USRLIB=		$(ROOT)/usr/lib
INC=		$(ROOT)/usr/include
INS=		install
LORDER=		lorder
TSORT=		tsort

DASHO=		-O
PICOPT=		-Kpic
CFLAGS=		$(DASHO) $(MORECPP) -DSYSV $(PICOPT) -I$(INC)


# "HOST" objects and libraries access /etc/hosts
# "DNS" objects and libraries try the domain name server, then /etc/hosts
# "ND" objects and libraries use the "netdir" generic interface
INETOBJS=	bindresvport.o byteorder.o ether_addr.o getnetbyaddr.o \
		getnetbyname.o getnetent.o getproto.o getprotoent.o \
		getprotoname.o getservent.o gtservbyname.o gtservbyport.o \
		inet_addr.o inet_lnaof.o inet_mkaddr.o inet_netof.o \
		inet_network.o rcmd.o rexec.o ruserpass.o inet_sethost.o \
		nd_gethost.o

SOCKOBJS=	accept.o bind.o callselect.o connect.o socket.o socketpair.o \
		shutdown.o getsockopt.o setsockopt.o listen.o \
		receive.o send.o _conn_util.o _utility.o \
		getsocknm.o getpeernm.o setsocknm.o setpeernm.o s_ioctl.o

OBJS=		$(INETOBJS) $(SOCKOBJS)
ARNAME=		libsocket.a
LIBNAME=	libsocket.so

PASSECHO=	\"CC=$(CC)\" \"DASHO=$(DASHO)\" \"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"MAKE=$(MAKE)\" \"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\"

PASSTHRU=	"CC=$(CC)" "DASHO=$(DASHO)" "FRC=$(FRC)" "INC=$(INC)" \
		"MAKE=$(MAKE)" "MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)"

all:
		@for i in inet socket;\
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
		rm -f $(ARNAME) $(LIBNAME)
		$(AR) crv $(ARNAME) `$(LORDER) $(OBJS) | $(TSORT)`
		if [ x$(CCSTYPE) != xCOFF ] ; \
		then \
		$(CC) -G -dy -ztext -o $(LIBNAME) $(OBJS) ;\
		fi

$(LIBNAME):	all

$(ARNAME):	all

install:	all
		cd ..;\
		wait
		if [ x$(CCSTYPE) != xCOFF ] ; \
		then \
		$(INS) -f $(USRLIB) -m 0444 -u root -g bin $(LIBNAME) ;\
		fi
		$(INS) -f $(USRLIB) -m 0444 -u root -g bin $(ARNAME)

clean:
		@for i in inet socket;\
		do\
			cd $$i;\
			echo "\n===== $(MAKE) -f $$i.mk clean $(PASSECHO)";\
			$(MAKE) -f $$i.mk clean $(PASSTHRU);\
			cd ..;\
		done;\
		wait
		-rm -f $(OBJS)

clobber:	clean
		-rm -f $(LIBNAME) $(ARNAME)
