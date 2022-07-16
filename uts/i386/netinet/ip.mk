#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-inet:ip.mk	1.3.1.1"
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
OIP = $(CONF)/pack.d/ip/Driver.o
OICMP = $(CONF)/pack.d/icmp/Driver.o
ORIP = $(CONF)/pack.d/rawip/Driver.o
PFLAGS = -DSYSV -Di386 $(MORECPP)
CPPFLAGS = -D_KERNEL $(PFLAGS) -I$(INCRT)
CFLAGS = -O $(CPPFLAGS)
CC = cc
LD = ld
FRC = 

PRODUCTS = IP ICMP RAWIP
OBJ = in.o in_cksum.o in_pcb.o in_switch.o in_transp.o ip_input.o \
	ip_output.o ip_main.o netlib.o route.o ip_vers.o
ROBJ=	raw_ip_main.o raw_ip.o raw_ip_cb.o
CLEAN = ip.o ip_icmp.o icmp.o $(OBJ) $(ROBJ)

all : $(PRODUCTS)

IP:	$(OIP)

ICMP:	$(OICMP)

RAWIP:	$(ORIP)

$(OIP):	$(OBJ)
	[ -d $(CONF)/pack.d/ip ] || mkdir $(CONF)/pack.d/ip; \
	$(LD) -r $(UTSLDFLAGS) -o $(OIP) $(OBJ)

$(ORIP):	$(ROBJ)
	[ -d $(CONF)/pack.d/rawip ] || mkdir $(CONF)/pack.d/rawip; \
	$(LD) -r $(UTSLDFLAGS) -o $(ORIP) $(ROBJ)

$(OICMP):	ip_icmp.o
	rm -f icmp.o
	ln ip_icmp.o icmp.o
	[ -d $(CONF)/pack.d/icmp ] || mkdir $(CONF)/pack.d/icmp
	$(LD) -r -o $(OICMP) icmp.o
	rm icmp.o

ip_icmp.o:	ip_icmp.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/icmp_var.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_icmp.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/types.h

ip_main.o:	ip_main.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h

in_transp.o:	in_transp.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h 

ip_input.o:	ip_input.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/insrem.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_icmp.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/netinet/tcp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/types.h

in.o:		in.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/net/af.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_systm.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/types.h

route.o:	route.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/af.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h

in_cksum.o:	in_cksum.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/types.h

in_pcb.o:	in_pcb.c \
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
	$(INCRT)/netinet/insrem.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/types.h

in_switch.o:	in_switch.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/af.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/route.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h

#insrem.o:	insrem.c \
	#$(INCRT)/netinet/insrem.h \
	#$(INCRT)/sys/socket.h \
	#$(INCRT)/sys/types.h

raw_ip_cb.o:	raw_ip_cb.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/net/route.h

raw_ip.o:	raw_ip.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/if.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/in_systm.h

raw_ip_main.o:	raw_ip_main.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/net/route.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/in_pcb.h \
	$(INCRT)/netinet/ip_var.h \
	$(INCRT)/netinet/in_var.h \
	$(INCRT)/netinet/ip_str.h \
	$(INCRT)/netinet/ip.h \
	$(INCRT)/netinet/in_systm.h


clean:
		rm -f $(CLEAN)

clobber:	clean
		rm -f $(ORIP) $(OIP) ($ICMP)

