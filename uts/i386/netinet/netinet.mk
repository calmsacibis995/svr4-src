#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-inet:netinet.mk	1.3.1.1"
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
CPPFLAGS = -D_KERNEL $(PFLAGS) -I$(INC)
CFLAGS = -O $(CPPFLAGS)
CC = cc
LD = ld
ARP = $(CONF)/pack.d/arp/Driver.o
APP = $(CONF)/pack.d/app/Driver.o
ICMP = $(CONF)/pack.d/icmp/Driver.o
IP = $(CONF)/pack.d/ip/Driver.o
LLCLOOP = $(CONF)/pack.d/llcloop/Driver.o
RAWIP = $(CONF)/pack.d/rawip/Driver.o
TCP = $(CONF)/pack.d/tcp/Driver.o
UDP = $(CONF)/pack.d/udp/Driver.o

PRODUCTS = $(ARP) $(APP) $(ICMP) $(IP) $(LLCLOOP) $(RAWIP) $(TCP) $(UDP)

uts: all

all:	$(PRODUCTS)

$(ARP):	FRC
	$(MAKE) -f arp.mk $(DEFS) ARP

$(APP):	FRC
	$(MAKE) -f arp.mk $(DEFS) APP

$(ICMP):	FRC
	$(MAKE) -f ip.mk $(DEFS) ICMP

$(IP):	FRC
	$(MAKE) -f ip.mk $(DEFS) IP

$(LLCLOOP):	FRC
	$(MAKE) -f llcloop.mk $(DEFS) LLCLOOP

$(RAWIP):	FRC
	$(MAKE) -f ip.mk $(DEFS) RAWIP

$(TCP):	FRC
	$(MAKE) -f tcp.mk $(DEFS) TCP

$(UDP):	FRC
	$(MAKE) -f udp.mk $(DEFS) UDP

clean:
	rm -f *.o

clobber:	clean
	rm -f $(PRODUCTS)

FRC:
