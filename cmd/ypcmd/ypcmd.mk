#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ypcmd:ypcmd.mk	1.6.7.1"

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
INC=$(ROOT)/usr/include
CFLAGS=-O -I$(INC) 
LDFLAGS=-s
LINKLIBS=-dy -lnsl 
BIN=ypmatch ypwhich ypcat 
SBIN=makedbm yppoll yppush ypset ypxfr ypalias 
 
SCRIPTS=ypinit ypxfr_1day ypxfr_2day ypxfr_1hour
YPSVC=ypbind ypserv ypupdated udpublickey
YPSERVOBJ=ypserv.o ypserv_ancil.o ypserv_map.o ypserv_proc.o \
	yp_getalias.o getlist.o
YPBINDOBJ=yp_b_svc.o yp_b_subr.o pong.o \
	 getlist.o yp_getalias.o
YPUPDOBJ=ypupdated.o openchild.o
INSTALL= install
BINDIR=$(ROOT)/usr/bin
SBINDIR=$(ROOT)/usr/sbin
NETSVC=$(ROOT)/usr/lib/netsvc
NETYP=$(ROOT)/usr/lib/netsvc/yp
ETC=		$(ROOT)/etc
VAR=		$(ROOT)/var
YP=		$(VAR)/yp
BINDINGS=	$(YP)/binding
INSTALL=	install
VARFILES=	./net_files/aliases ./net_files/updaters ./net_files/Makefile

all:	$(BIN) $(SBIN) $(SCRIPTS) $(YPSVC) $(ETCFILES) $(VARFILES)

ypserv:	$(YPSERVOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(YPSERVOBJ) $(LDLIBS) $(LINKLIBS)
	
ypbind:	$(YPBINDOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(YPBINDOBJ) $(LDLIBS) $(LINKLIBS)

ypupdated:	$(YPUPDOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(YPUPDOBJ) $(LDLIBS) $(LINKLIBS)

makedbm yppush: $$@.o yp_getalias.o getlist.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.o yp_getalias.o getlist.o $(LDLIBS) $(LINKLIBS)

ypxfr: $$@.o yp_getalias.o getlist.o 
	$(CC) -c -I$(INC) ypalias.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.o ypalias.o yp_getalias.o getlist.o $(LDLIBS) $(LINKLIBS)
	rm ypalias.o

ypalias: ypalias.o yp_getalias.o getlist.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.o yp_getalias.o getlist.o $(LDLIBS) $(LINKLIBS)

$(BIN) yppoll ypset: $$@.o yp_getalias.o getlist.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.o yp_getalias.o getlist.o $(LDLIBS) $(LINKLIBS)

getlist.o:	getlist.c \
	$(INC)/stdio.h	

makedbm.o:	makedbm.c \
	$(INC)/rpcsvc/dbm.h	\
	$(INC)/stdio.h	\
	$(INC)/sys/types.h	\
	$(INC)/sys/file.h	\
	$(INC)/sys/param.h	\
	$(INC)/sys/stat.h	\
	$(INC)/ctype.h	\
	ypdefs.h	

openchild.o:	openchild.c \
	$(INC)/stdio.h	

pong.o:	pong.c \
	$(INC)/rpc/rpc.h	\
	yp_b.h	\
	$(INC)/rpcsvc/yp_prot.h	

yp_b_subr.o:	yp_b_subr.c \
	$(INC)/signal.h	\
	$(INC)/stdio.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	$(INC)/dirent.h	\
	$(INC)/sys/wait.h \
	yp_b.h	

yp_b_svc.o:	yp_b_svc.c \
	$(INC)/stdio.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/rpc/nettype.h	\
	yp_b.h	\
	$(INC)/netconfig.h	

ypcat.o:	ypcat.c \
	$(INC)/stdio.h	\
	$(INC)/rpc/rpc.h	\
	yp_b.h	\
	$(INC)/rpcsvc/ypclnt.h	\
	$(INC)/rpcsvc/yp_prot.h	

ypmatch.o:	ypmatch.c \
	$(INC)/stdio.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/sys/socket.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	$(INC)/rpcsvc/ypclnt.h	

yppoll.o:	yppoll.c \
	$(INC)/stdio.h	\
	$(INC)/ctype.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/sys/socket.h	\
	$(INC)/rpcsvc/ypclnt.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	yp_b.h	

yppush.o:	yppush.c \
	$(INC)/stdio.h	\
	$(INC)/errno.h	\
	$(INC)/signal.h	\
	$(INC)/sys/types.h	\
	$(INC)/sys/wait.h	\
	$(INC)/sys/stat.h	\
	$(INC)/ctype.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/rpc/rpcb_prot.h	\
	$(INC)/rpc/rpcb_clnt.h	\
	$(INC)/rpcsvc/ypclnt.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	yp_b.h	\
	ypdefs.h

ypserv.o:	ypserv.c \
	ypsym.h	\
	$(INC)/sys/ioctl.h	\
	$(INC)/sys/file.h	\
	ypdefs.h	

ypserv_ancil.o:	ypserv_ancil.c \
	ypsym.h	\
	ypdefs.h	\
	$(INC)/dirent.h	

ypserv_map.o:	ypserv_map.c \
	ypsym.h	\
	ypdefs.h	\
	$(INC)/dirent.h	\
	$(INC)/ctype.h	

ypserv_proc.o:	ypserv_proc.c \
	ypsym.h	\
	ypdefs.h	\
	$(INC)/ctype.h	

ypset.o:	ypset.c \
	$(INC)/stdio.h	\
	$(INC)/ctype.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/rpcsvc/ypclnt.h	\
	yp_b.h	\
	$(INC)/rpcsvc/yp_prot.h	

ypupdated.o:	ypupdated.c \
	$(INC)/stdio.h	\
	$(INC)/sys/types.h	\
	$(INC)/sys/file.h	\
	$(INC)/sys/wait.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/rpc/auth_des.h	\
	$(INC)/rpc/nettype.h	\
	$(INC)/rpcsvc/ypupd.h	\
	$(INC)/rpcsvc/ypclnt.h	

ypwhich.o:	ypwhich.c \
	$(INC)/stdio.h	\
	$(INC)/ctype.h	\
	$(INC)/rpc/rpc.h	\
	yp_b.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	ypv2_bind.h	\
	$(INC)/rpcsvc/ypclnt.h	

ypxfr.o:	ypxfr.c \
	$(INC)/rpcsvc/dbm.h	\
	$(INC)/stdio.h	\
	$(INC)/errno.h	\
	$(INC)/ctype.h	\
	$(INC)/dirent.h	\
	$(INC)/rpc/rpc.h	\
	$(INC)/sys/socket.h	\
	$(INC)/sys/dir.h	\
	$(INC)/sys/file.h	\
	$(INC)/sys/stat.h	\
	$(INC)/rpcsvc/ypclnt.h	\
	ypdefs.h	\
	$(INC)/rpcsvc/yp_prot.h	\
	yp_b.h

ypalias.o: ypalias.c \
	$(INC)/stdio.h	\
	$(INC)/string.h	\
	$(INC)/limits.h	\
	$(INC)/sys/types.h	\
	$(INC)/sys/statvfs.h	\
	ypsym.h
	$(CC) -c -I$(INC) -DMAIN ypalias.c

install: all
	if [ ! -d $(NETSVC) ] ; \
	then \
		mkdir $(NETSVC) ; \
	fi ; \
	if [ ! -d $(NETYP) ] ; \
	then \
		mkdir $(NETYP) ; \
	fi ; \
	$(INSTALL) -f $(BINDIR) ypmatch
	$(INSTALL) -f $(BINDIR) ypcat
	$(INSTALL) -f $(BINDIR) ypwhich
	$(INSTALL) -f $(NETYP) ypserv
	$(INSTALL) -f $(SBINDIR) ypalias
	$(INSTALL) -f $(SBINDIR) makedbm 
	$(INSTALL) -f $(SBINDIR) yppoll
	$(INSTALL) -f $(SBINDIR) yppush
	$(INSTALL) -f $(SBINDIR) ypset
	$(INSTALL) -f $(SBINDIR) ypxfr
	$(INSTALL) -f $(SBINDIR) ypinit
	$(INSTALL) -f $(NETYP) ypbind 
	$(INSTALL) -f $(NETYP) ypupdated
	$(INSTALL) -f $(SBINDIR) udpublickey
	if [ ! -d $(YP) ] ; \
	then \
		mkdir $(YP) ; \
	fi ; \
	if [ ! -d $(BINDINGS) ] ; \
	then \
		mkdir $(BINDINGS) ; \
	fi ; \
	for i in $(VARFILES) ; \
	do \
		$(INSTALL) -f $(YP) -m 0444 -u root -g sys $$i ; \
	done 

clobber:	clean

clean:	
	rm -f getlist.o makedbm.o openchild.o \
	$(YPSERVOBJ) $(YPBINDOBJ) ypcat.o ypmatch.o yppoll.o \
	ypupdated.o ypwhich.o ypxfr.o yppush.o ypset.o
	rm $(BIN) $(SBIN) $(SCRIPTS) $(YPSVC)

size:	$(BIN) $(SBIN)
	$(SIZE) $(BIN) $(SBIN)

strip:	$(BIN) $(SBIN)
	$(STRIP) $(BIN) $(SBIN)

lint:
	lint $(YPSERVOBJ:.o=.c) 
	lint $(YPBINDOBJ:.o=.c)
	lint $(YPUPDOBJ:.o=.c)
	lint ypmatch.c
	lint ypwhich.c
	lint ypcat.c
	lint yppoll.c
	lint yppush.c
	lint ypset.c
	lint ypxfr.c yp_getalias.c getlist.c
	lint makedbm.c yp_getalias.c getlist.c

