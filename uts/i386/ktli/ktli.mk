#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-ktli:ktli.mk	1.1"
#
#	@(#)ktli.mk 1.2 89/01/11 SMI
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
#
#	Kernel TLI inteface
#
STRIP = strip
INCRT = ..
OFILE = $(CONF)/pack.d/ktli/Driver.o
PFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -DSYSV
DEFLIST =
FRC =

KTLIOBJ = t_kclose.o t_kgtstate.o t_ksndudat.o t_kutil.o t_kalloc.o \
	  t_kconnect.o t_kopen.o t_kspoll.o t_kbind.o t_kunbind.o t_kfree.o \
	  t_krcvudat.o 

all:	$(OFILE)

lint:
	lint $(CFLAGS) -Dlint *.c

$(OFILE):	$(KTLIOBJ)
	[ -d $(CONF)/pack.d/ktli ] || mkdir $(CONF)/pack.d/ktli; \
	$(LD) -r -o $(OFILE) $(KTLIOBJ)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILE)


#
# Header dependencies -- make depend should build these!
#

t_kalloc.o: t_kalloc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

t_kbind.o: t_kbind.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

t_kunbind.o: t_kunbind.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

t_kclose.o: t_kclose.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kconnect.o: t_kconnect.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kfree.o: t_kfree.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kgtstate.o: t_kgtstate.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kopen.o: t_kopen.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

t_krcvudat.o: t_krcvudat.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_ksndudat.o: t_ksndudat.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kspoll.o: t_kspoll.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)

t_kutil.o: t_kutil.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(FRC)
