#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-inet:arp.mk	1.3.1.1"
#
#
#  		PROPRIETARY NOTICE (Combined)
#  
#  This source code is unpublished proprietary information
#  constituting, or derived under license from AT&T's Unix(r) System V.
#  In addition, portions of such source code were derived from Berkeley
#  4.3 BSD under license from the Regents of the University of
#  California.
#  
#  
#  
#  		Copyright Notice 
#  
#  Notice of copyright on this source code product does not indicate 
#  publication.
#  
#  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
#  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#  	          All rights reserved.
#
INCRT = ..
PFLAGS = -DSYSV -Di386 $(MORECPP)
CPPFLAGS = -D_KERNEL $(PFLAGS) -I$(INCRT)
CFLAGS = -O $(CPPFLAGS)
OARP = $(CONF)/pack.d/arp/Driver.o
OAPP = $(CONF)/pack.d/app/Driver.o

PRODUCTS = ARP APP
CLEAN = arp.o app.o


all:	ARP APP

ARP:	arp.o
	[ -d $(CONF)/pack.d/arp ] || mkdir $(CONF)/pack.d/arp; \
	$(LD) -r -o $(OARP) arp.o

APP:	app.o
	[ -d $(CONF)/pack.d/app ] || mkdir $(CONF)/pack.d/app; \
	$(LD) -r -o $(OAPP) app.o

arp.o:		arp.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/if_ether.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h

clean:
		rm -f $(CLEAN)

clobber:	clean
		rm -f $(OARP) $(OAPP)
