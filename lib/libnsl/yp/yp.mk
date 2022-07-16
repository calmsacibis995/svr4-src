#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)libyp:libyp.mk	1.10.1.1"

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
#
USRLIB = $(ROOT)/usr/lib
INC = $(ROOT)/usr/include 
CFLAGS= -O -Kpic -I$(INC) -D_NSL_RPC_ABI
STRIP = strip
SIZE = size
LINT = lint
BINDOBJS= yp_b_clnt.o yp_b_xdr.o
OBJS= dbm.o yp_all.o yp_bind.o yp_enum.o yp_master.o yp_match.o yp_order.o \
	yp_update.o ypupd.o yperr_string.o ypprot_err.o ypxdr.o ypmaint_xdr.o\
	$(BINDOBJS)

LIBOBJS= ../dbm.o ../yp_all.o ../yp_bind.o ../yp_enum.o\
	../yp_master.o ../yp_match.o ../yp_order.o \
	../yp_update.o ../ypupd.o ../yperr_string.o ../ypprot_err.o \
	../ypxdr.o ../ypmaint_xdr.o\
	../yp_b_clnt.o ../yp_b_xdr.o

YPSRC= $(OBJS:.o=.c)
HDRS = dbm.h yp_prot.h ypclnt.h  

all: $(OBJS)
	cp $(OBJS) ../

dbm.o: dbm.c\
	$(INC)/rpcsvc/dbm.h\
	$(INC)/sys/types.h\
	$(INC)/sys/stat.h
 
yp_all.o: yp_all.c\
	$(INC)/rpc/rpc.h\
	$(INC)/sys/syslog.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_b_clnt.o: yp_b_clnt.c\
	$(INC)/rpc/rpc.h\
	$(INC)/sys/time.h\
	yp_b.h
 
yp_b_xdr.o: yp_b_xdr.c\
	$(INC)/rpc/rpc.h\
	yp_b.h
 
yp_bind.o: yp_bind.c\
	$(INC)/stdio.h\
	$(INC)/errno.h\
	$(INC)/unistd.h\
	$(INC)/rpc/rpc.h\
	$(INC)/sys/syslog.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_enum.o: yp_enum.c\
	$(INC)/rpc/rpc.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_master.o: yp_master.c\
	$(INC)/rpc/rpc.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_match.o: yp_match.c\
	$(INC)/rpc/rpc.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_order.o: yp_order.c\
	$(INC)/rpc/rpc.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
yp_update.o: yp_update.c\
	$(INC)/stdio.h\
	$(INC)/rpc/rpc.h\
	$(INC)/rpcsvc/ypupd.h\
	yp_b.h

yperr_string.o: yperr_string.c\
	$(INC)/rpcsvc/ypclnt.h
 
ypmaint_xdr.o: ypmaint_xdr.c\
	$(INC)/rpc/rpc.h\
	yp_b.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
ypprot_err.o: ypprot_err.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 
ypupd.o: ypupd.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpcsvc/ypupd.h
 
ypxdr.o: ypxdr.c\
	$(INC)/rpc/rpc.h\
	$(INC)/rpcsvc/yp_prot.h\
	$(INC)/rpcsvc/ypclnt.h
 

lint:
	$(LINT) $(CFLAGS) $(YPSRCS) 

clean:
	rm -f $(OBJS) 

clobber: clean
	rm -f $(LIBOBJS)

strip:	all
	$(STRIP) $(LIBOBJS)

size:	all
	$(SIZE) $(LIBOBJS)
