#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-rpc:rpc.mk	1.3.1.1"
#
#	@(#)rpc.mk 1.4 89/01/03 SMI
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
#	Kernel RPC
#
STRIP = strip
INCRT = ..
OFILE = $(CONF)/pack.d/krpc/Driver.o
PFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -DSYSV
DEFLIST =
FRC =

KRPCOBJ = clnt_clts.o clnt_gen.o svc_gen.o svc_clts.o \
	  xdr_mblk.o xdr_mem.o svc.o  auth_kern.o rpc_prot.o \
	  rpc_calmsg.o xdr.o svc_auth.o authu_prot.o \
	  svcauthdes.o svc_authu.o xdr_array.o key_call.o \
	  key_prot.o clnt_perr.o \
	  auth_des.o authdesprt.o authdesubr.o rpc_subr.o

all:	$(OFILE)

lint:
	lint $(CFLAGS) -Dlint *.c

$(OFILE):	$(KRPCOBJ)
	[ -d $(CONF)/pack.d/krpc ] || mkdir $(CONF)/pack.d/krpc; \
	$(LD) -r -o $(OFILE) $(KRPCOBJ)

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(OFILE)

#
# Header dependencies -- make depend should build these!
#

auth_des.o: auth_des.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/rpc/des_crypt.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_des.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

auth_kern.o: auth_kern.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_unix.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

authdesprt.o: authdesprt.c \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_des.h \
	$(FRC)

authdesubr.o: authdesubr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/rpc.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/systeminfo.h \
	$(FRC)

authu_prot.o: authu_prot.c \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_unix.h \
	$(INCRT)/sys/utsname.h \
	$(FRC)

clnt_clts.o: clnt_clts.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

clnt_gen.o: clnt_gen.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(FRC)

clnt_perr.o: clnt_perr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(FRC)

key_call.o: key_call.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/rpc/rpc.h \
	$(INCRT)/rpc/key_prot.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

key_prot.o: key_prot.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/rpc.h \
	$(INCRT)/rpc/key_prot.h \
	$(FRC)

rpc_calmsg.o: rpc_calmsg.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/netinet/in.h \
	$(FRC)

rpc_prot.o: rpc_prot.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/netinet/in.h \
	$(FRC)

svc.o: svc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/sys/siginfo.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/svc_auth.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

svc_auth.o: svc_auth.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/svc_auth.h \
	$(FRC)

svc_authu.o: svc_authu.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/auth_unix.h \
	$(INCRT)/rpc/svc_auth.h \
	$(FRC)

svc_clts.o: svc_clts.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
	$(FRC)

svc_gen.o: svc_gen.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/clnt.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

svcauthdes.o: svcauthdes.c \
	$(INCRT)/rpc/des_crypt.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/time.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/rpc/auth.h \
	$(INCRT)/rpc/auth_des.h \
	$(INCRT)/rpc/svc_auth.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/t_kuser.h \
	$(INCRT)/rpc/svc.h \
	$(INCRT)/rpc/rpc_msg.h \
	$(FRC)

xdr.o: xdr.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(FRC)

xdr_array.o: xdr_array.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(FRC)

xdr_mblk.o: xdr_mblk.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/rpc/xdr.h \
	$(INCRT)/netinet/in.h \
	$(FRC)

xdr_mem.o: xdr_mem.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/netinet/in.h \
	$(INCRT)/rpc/types.h \
	$(INCRT)/rpc/xdr.h \
	$(FRC)

rpc_subr.o: rpc_subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/rpc/types.h \
	$(FRC)
