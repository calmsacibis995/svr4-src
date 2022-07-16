#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)librpc:librpc.mk	1.9.7.1"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
# 
# Makefile for user level rpc library

#
# Kernel level sources are missing here. Only USER
#	level rpc sources here
#
# Sources deleted because of duplicate or obsolete functionality
#	clnt_udp.c\ clnt_tcp.c
#	get_myaddress.c\
#	pmap_getport.c\ pmap_getmaps.c
#	pmap_prot2.c\ pmap_rmt.c
#	rpc_dtbsize.c\
#	svc_tcp.c\ svc_udp.c
#
# Sources renamed due to file name length and copyright infringement problems:
#	authunix_prot.c\ ==> authsys_prot.c
#	svc_auth_unix.c\ ==>svc_auth_sys.c
#	xdr_reference.c\ ==> xdr_refer.c
#	auth_unix.h ==> auth_sys.h
#
# New sources added:
#	clnt_bcast.c\: clnt_broadcast here.
#	clnt_dg.c\: All datagram support for the clients
#	clnt_vc.c\: All connectionful (virtual circuit) support for the clients
#	clnt_generic.c\ : All the new tli stuff for client side
#	rpc_soc.c\ : All the socket compatibility stuff
#	rpc_generic.c\ : generic routines used by both svc and client
#	rpcb_prot.c\: All the new rpcbind protocol xdr routines here
#	rpcb_clnt.h: All the rpcbind client side interfaces.
#	svc_dg.c\: All datagram support for the servers
#	svc_vc.c\: All connectionful (virtual circuit) support for the servers
#	svc_generic.c\ : All the new interface routines for svc side
#
# Sources in the compatibility package
#	clnt_soc.h
#	dbx_rpc.c\
#	pmap_clnt.c\
#	pmap_prot.c\
#	port.c\
#	rpc_soc.c\
#	svc_soc.h
#
# Source in the secure package
#	auth_des.c\
#	authdes_prot.c\
#	rpcdname.c\
#	rtime_tli.c\
#	svcdesname.c\
#	svc_authdes.c\
#
# Headers included in component head.usrs:
# HDRS = auth.h auth_des.h auth_unix.h auth_sys.h clnt.h clnt_soc.h \
#	keyprot.h \
#	nettype.h pmap_clnt.h pmap_prot.h pmap_rmt.h raw.h \
#	rpc.h rpc_com.h rpcent.h rpc_msg.h rpcb_clnt.h rpcb_prot.h \
#	svc.h svc_soc.h svc_auth.h types.h xdr.h


USRLIB	= $(ROOT)/usr/lib
LINT	= lint
STRIP	= strip
SIZE	= size
INS	= install
INC	= $(ROOT)/usr/include 
CPPFLAGS = -O -Kpic -I$(INC) -DPORTMAP -D_NSL_RPC_ABI -DYP
CFLAGS	= $(CPPFLAGS) 

HDRS =	auth.h auth_des.h auth_sys.h auth_unix.h clnt.h clnt_soc.h \
	key_prot.h nettype.h \
	pmap_clnt.h pmap_prot.h pmap_rmt.h raw.h \
	rpc.h rpc_com.h rpcent.h rpc_msg.h rpcb_clnt.h rpcb_prot.h \
	svc.h svc_soc.h svc_auth.h types.h xdr.h

SECOBJS=auth_des.o authdes_prot.o getdname.o key_prot.o key_call.o \
	netname.o netnamer.o openchild.o rpcdname.o rtime_tli.o\
	svcauth_des.o svcdesname.o

INETOBJS=gethostent.o inet_ntoa.o

OBJS =	$(SECOBJS) $(INETOBJS)  auth_none.o auth_sys.o authsys_prot.o \
	clnt_perror.o clnt_raw.o clnt_simple.o \
	clnt_dg.o clnt_vc.o clnt_generic.o clnt_bcast.o getdname.o \
	gethostname.o getrpcent.o \
	pmap_clnt.o pmap_prot.o port.o publickey.o \
	rpc_callmsg.o rpc_comdata.o rpcdname.o\
	rpc_generic.o rpc_prot.o rpc_soc.o \
	rpcb_clnt.o rpcb_prot.o secretkey.o ti_opts.o\
	svc.o svc_auth.o svc_auth_sys.o svc_dg.o \
	svc_vc.o svc_generic.o svc_raw.o svc_run.o svc_simple.o \
	xcrypt.o xdr.o xdr_array.o xdr_mbuf.o xdr_float.o\
	xdr_mem.o xdr_rec.o xdr_refer.o xdr_stdio.o 

LIBOBJS= ../auth_des.o ../authdes_prot.o ../getdname.o ../key_prot.o ../key_call.o\
	../netname.o ../netnamer.o ../openchild.o ../rpcdname.o ../rtime_tli.o \
	../svcauth_des.o ../svcdesname.o \
	../auth_none.o ../auth_sys.o ../authsys_prot.o \
	../clnt_bcast.o ../clnt_generic.o \
	../clnt_dg.o ../clnt_vc.o \
	../clnt_perror.o ../clnt_raw.o ../clnt_simple.o \
	../getdname.o \
	../gethostname.o ../getrpcent.o \
	../pmap_clnt.o ../pmap_prot.o ../port.o ../publickey.o \
	../rpc_callmsg.o ../rpc_comdata.o ../rpcdname.o \
	../rpc_generic.o ../rpc_prot.o ../rpc_soc.o \
	../rpcb_clnt.o ../rpcb_prot.o ../secretkey.o ../ti_opts.o\
	../svc.o ../svc_auth.o ../svc_auth_sys.o ../svc_dg.o \
	../svc_vc.o ../svc_generic.o ../svc_raw.o ../svc_run.o ../svc_simple.o \
	../xcrypt.o ../xdr.o ../xdr_array.o ../xdr_mbuf.o ../xdr_float.o \
	../xdr_mem.o ../xdr_rec.o ../xdr_refer.o ../xdr_stdio.o \
	../gethostent.o ../inet_ntoa.o


RPCINCLUDES = $(HDRS:%=$(INC)/rpc/%)

SRCS = $(OBJS:.o=.c\)

all: $(OBJS)
	cp $(OBJS) ../

gethostent.o: gethostent.c \
	$(INC)/stdio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/socket.h \
	$(INC)/netdb.h \
	$(INC)/ctype.h 

inet_ntoa.o: inet_ntoa.c \
	$(INC)/sys/types.h \
	$(INC)/ctype.h \
	$(INC)/netinet/in.h

auth_des.o: auth_des.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/des_crypt.h
 
auth_none.o: auth_none.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h
 
auth_sys.o: auth_sys.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/auth_sys.h
 
authdes_prot.o: authdes_prot.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/auth_des.h
 
authsys_prot.o: authsys_prot.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/auth_sys.h
 
clnt_bcast.o: clnt_bcast.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/pmap_prot.h\
	$(INC)/rpc/pmap_clnt.h\
	$(INC)/rpc/pmap_rmt.h\
	$(INC)/rpc/nettype.h\
	$(INC)/stdio.h\
	$(INC)/errno.h
 
clnt_dg.o: clnt_dg.c\
	$(INC)/rpc/rpc.h\
	$(INC)/errno.h
 
clnt_generic.o: clnt_generic.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/nettype.h
 
clnt_perror.o: clnt_perror.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/clnt.h
 
clnt_raw.o: clnt_raw.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/raw.h
 
clnt_simple.o: clnt_simple.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/string.h
 
clnt_vc.o: clnt_vc.c\
	$(INC)/rpc/rpc.h\
	$(INC)/errno.h\
	$(INC)/sys/byteorder.h
 
getdname.o: getdname.c\
	$(INC)/stdio.h
 
getrpcent.o: getrpcent.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpcent.h
 
key_call.o: key_call.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/key_prot.h\
	$(INC)/signal.h\
	$(INC)/stdio.h\
	$(INC)/sys/wait.h
 
key_prot.o: key_prot.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/key_prot.h
 
netname.o: netname.c\
	$(INC)/rpc/rpc.h\
	$(INC)/ctype.h
 
 
netnamer.o: netnamer.c\
	$(INC)/rpc/rpc.h\
	$(INC)/ctype.h\
	$(INC)/stdio.h\
	$(INC)/grp.h\
	$(INC)/pwd.h\
	$(INC)/string.h
 
openchild.o: openchild.c\
	$(INC)/stdio.h
 
pmap_clnt.o: pmap_clnt.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/nettype.h\
	$(INC)/netdir.h\
	$(INC)/rpc/pmap_prot.h\
	$(INC)/rpc/pmap_clnt.h\
	$(INC)/rpc/pmap_rmt.h\
	$(INC)/netinet/in.h
 
pmap_prot.o: pmap_prot.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/pmap_prot.h\
	$(INC)/rpc/pmap_rmt.h

port.o: port.c\
	$(INC)/rpc/rpc.h\
	$(INC)/stdio.h
 
publickey.o: publickey.c\
	$(INC)/rpc/rpc.h\
	$(INC)/stdio.h\
	$(INC)/pwd.h\
	$(INC)/rpc/key_prot.h\
	$(INC)/string.h

rpc_callmsg.o: rpc_callmsg.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/clnt.h\
	$(INC)/rpc/rpc_msg.h\
	$(INC)/rpc/rpc.h\
	$(INC)/sys/byteorder.h
 
rpc_comdata.o: rpc_comdata.c\
	$(INC)/rpc/rpc.h
 
rpc_generic.o: rpc_generic.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/nettype.h\
	$(INC)/ctype.h
 
rpc_prot.o: rpc_prot.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/clnt.h\
	$(INC)/rpc/rpc_msg.h\
	$(INC)/rpc/rpc.h
 
rpc_soc.o: rpc_soc.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/netinet/in.h\
	$(INC)/netdb.h\
	$(INC)/errno.h\
	$(INC)/rpc/pmap_clnt.h\
	$(INC)/rpc/pmap_prot.h\
	$(INC)/rpc/nettype.h
 
rpcb_clnt.o: rpcb_clnt.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/rpcb_prot.h\
	$(INC)/netconfig.h\
	$(INC)/netdir.h\
	$(INC)/netinet/in.h\
	$(INC)/stdio.h
 
rpcb_prot.o: rpcb_prot.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/rpcb_prot.h

secretkey.o: secretkey.c\
	$(INC)/stdio.h\
	$(INC)/pwd.h\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/key_prot.h\
	$(INC)/string.h

ti_opts.o: ti_opts.c\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/tiuser.h\
	$(INC)/rpc/rpc.h\
	$(INC)/sys/ticlts.h

rtime_tli.o: rtime_tli.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/nettype.h\
	$(INC)/netdir.h\
	$(INC)/stdio.h
 
svc.o: svc.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/pmap_clnt.h\
	$(INC)/netconfig.h
 
svc_auth.o: svc_auth.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/clnt.h\
	$(INC)/rpc/rpc_msg.h\
	$(INC)/rpc/svc.h\
	$(INC)/rpc/svc_auth.h\
	$(INC)/rpc/rpc.h
 
svc_auth_sys.o: svc_auth_sys.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/rpc/auth.h\
	$(INC)/rpc/clnt.h\
	$(INC)/rpc/rpc_msg.h\
	$(INC)/rpc/svc.h\
	$(INC)/rpc/auth_sys.h\
	$(INC)/rpc/svc_auth.h\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h
 
svc_dg.o: svc_dg.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/errno.h
 
svc_generic.o: svc_generic.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/errno.h\
	$(INC)/rpc/nettype.h
 
svc_raw.o: svc_raw.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/raw.h
 
svc_run.o: svc_run.c\
	$(INC)/rpc/rpc.h
 
svc_simple.o: svc_simple.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/rpc/nettype.h
 
svc_vc.o: svc_vc.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/errno.h\
	$(INC)/rpc/nettype.h
 
svcauth_des.o: svcauth_des.c\
	$(INC)/rpc/des_crypt.h\
	$(INC)/rpc/rpc.h
 
svcdesname.o: svcdesname.c\
	$(INC)/rpc/des_crypt.h\
	$(INC)/rpc/rpc.h
 
xcrypt.o: xcrypt.c\
	$(INC)/stdio.h\
	$(INC)/rpc/des_crypt.h\
	$(INC)/memory.h

xdr.o: xdr.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h
 
xdr_array.o: xdr_array.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/memory.h
 
xdr_float.o: xdr_float.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h
 
xdr_mbuf.o: xdr_mbuf.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h
 
xdr_mem.o: xdr_mem.c\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/memory.h
 
xdr_rec.o: xdr_rec.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/memory.h
 
xdr_refer.o: xdr_refer.c\
	$(INC)/stdio.h\
	$(INC)/rpc/types.h\
	$(INC)/rpc/xdr.h\
	$(INC)/memory.h
 
xdr_stdio.o: xdr_stdio.c\
	$(INC)/rpc/types.h\
	$(INC)/stdio.h\
	$(INC)/rpc/xdr.h
 
lint:
	$(LINT) $(CFLAGS) $(SRCS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(LIBOBJS)

strip:	all
	$(STRIP) $(LIBOBJS)

size:	all
	$(SIZE) $(LIBOBJS)
