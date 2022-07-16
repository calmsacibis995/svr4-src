#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-inet:udp.mk	1.3.1.1"
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
OUDP = $(CONF)/pack.d/udp/Driver.o
PFLAGS = -DSYSV -Di386 $(MORECPP)
CPPFLAGS = -D_KERNEL $(PFLAGS) -I$(INCRT)
CFLAGS = -O $(CPPFLAGS)
CC = cc
LD = ld

PRODUCTS = UDP
OBJ = udp_io.o udp_main.o udp_state.o 
CLEAN = udp.o $(OBJ)

all : $(PRODUCTS)

UDP: $(OUDP)

$(OUDP): $(OBJ)
	[ -d $(CONF)/pack.d/udp ] || mkdir $(CONF)/pack.d/udp; \
	$(LD) -r $(UTSLDFLAGS) -o $(OUDP) $(OBJ)

udp_main.o:	udp_main.c \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/types.h

udp_state.o:	udp_state.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_icmp.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/netinet/udp.h \
	$(INCRT)/netinet/udp_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/types.h

udp_io.o:	udp_io.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_icmp.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/netinet/udp.h \
	$(INCRT)/netinet/udp_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/types.h

clean:
		rm -f $(CLEAN)

clobber:	clean
		rm -f $(OUDP)
