#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)nametoaddr:resolv/libsocket/inet/inet.mk	1.1.2.1"

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

INC=		$(ROOT)/usr/include

DASHO=		-O
PICOPT=		-Kpic
CFLAGS=		$(DASHO) $(MORECPP) -DSYSV $(PICOPT) -I$(INC) -D_RESOLV_ABI

OBJS=		bindresvport.o byteorder.o ether_addr.o getnetbyaddr.o \
		getnetbyname.o getnetent.o getproto.o getprotoent.o \
		getprotoname.o getservent.o gtservbyname.o gtservbyport.o \
		inet_addr.o inet_lnaof.o inet_mkaddr.o inet_netof.o \
		inet_network.o rcmd.o rexec.o ruserpass.o

all:		$(OBJS)
		cp $(OBJS) ../

install:	all

clean:
		-rm -f *.o

clobber:	clean

#
# to do -- Header dependencies
#
