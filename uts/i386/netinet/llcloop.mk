#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-inet:llcloop.mk	1.3.1.1"
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
OLLCLOOP = $(CONF)/pack.d/llcloop/Driver.o
PFLAGS = -DSYSV -Di386 $(MORECPP)
CPPFLAGS = -D_KERNEL $(PFLAGS) -I$(INCRT)
CFLAGS = -O $(CPPFLAGS)
CC = cc
LD = ld
FRC = 

PRODUCTS = LLCLOOP
CLEAN = llcloop.o

uts: all

all : $(PRODUCTS)

LLCLOOP:	llcloop.o
	[ -d $(CONF)/pack.d/llcloop ] || mkdir $(CONF)/pack.d/llcloop; \
	$(LD) -r -o $(OLLCLOOP) llcloop.o

llcloop.o:	llcloop.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/lihdr.h \
	$(INCRT)/netinet/llcloop.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/net/if.h \
	$(INCRT)/net/if_arp.h \
	$(INCRT)/net/strioc.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/netinet/in_var.h \
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

clean:
	rm -f $(CLEAN)

clobber:	clean
		rm -f $(OLLCLOOP)

