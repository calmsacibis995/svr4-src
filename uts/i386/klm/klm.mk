#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-klm:klm.mk	1.3"
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
ROOT =
STRIP = strip
INCRT = ..
OFILE=$(CONF)/pack.d/klm/Driver.o

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL -DSYSV $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
DEFLIST =
FRC =

FILES = \
	klm_kprot.o \
	klm_lkmgr.o

all:	KLM

KLM:	klm.o 

klm.o:	$(FILES)
	[ -d $(CONF)/pack.d/klm ] || mkdir $(CONF)/pack.d/klm; \
	$(LD) -r -o $(OFILE) $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies
#

klm_kprot.o: klm_kprot.c \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/rpc/auth_sys.h \
	$(INCRT)/rpc/auth_des.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/svc_auth.h \
	klm_prot.h \
	$(FRC)

klm_lkmgr.o: klm_lkmgr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/rpc/pmap_prot.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/klm/lockmgr.h \
	klm_prot.h \
	$(INCRT)/net/if.h \
	$(INCRT)/nfs/nfs.h \
	$(INCRT)/nfs/nfs_clnt.h \
	$(INCRT)/nfs/rnode.h \
	$(FRC)
