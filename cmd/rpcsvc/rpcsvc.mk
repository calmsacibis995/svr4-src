#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rpcsvc:rpcsvc.mk	1.25.1.1"

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
LINKLIBS	= -lrpcsvc -lnsl
DESTSERVERS	= $(ROOT)/usr/sbin
DESTCLIENTS	= $(ROOT)/usr/bin
NETSVC		= $(ROOT)/usr/lib/netsvc
RWALL		= $(ROOT)/usr/lib/netsvc/rwall
RUSERS		= $(ROOT)/usr/lib/netsvc/rusers
SPRAY		= $(ROOT)/usr/lib/netsvc/spray
ETC		=$(ROOT)/etc
NET		=$(ETC)/net
TICLTS		=$(NET)/ticlts
TICOTS		=$(NET)/ticots
TICOTSORD	=$(NET)/ticotsord
DIRS		= \
		$(NETSVC) \
		$(RWALL) \
		$(RUSERS) \
		$(SPRAY) \
		$(NET) \
		$(TICLTS) \
		$(TICOTS) \
		$(TICOTS) \
		$(TICOTSORD) 
INC		= $(ROOT)/usr/include 
CPPFLAGS	= -O -I$(INC)
CFLAGS		= $(CPPFLAGS)
LINT		= lint
INS		= install
LDFLAGS		= -dy -s

FILES=	./net_files/hosts ./net_files/services
ETCFILES= ./net_files/publickey ./net_files/netid ./net_files/rpc ./net_files/masters ./net_files/slaves 
SCLNTOBJS= spray.o spray_clnt.o
SSVCOBJS= spray_subr.o spray_svc.o
ROBJS	= rpc.rusersd.o
WSVCOBJS= rwall_subr.o rwall_svc.o
WCLNTOBJS = rwall.o rwall_clnt.o
OBJS	= $(SSVCOBJS) $(SCLNTOBJS) $(ROBJS) $(WCLNTOBJS) $(WSVCOBJS)
SRCS	= $(OBJS:.o=.c)

SERVERS = rpc.rwalld rpc.sprayd rpc.rusersd
CLIENTS = rusers rwall spray 
GOAL	= $(SERVERS) $(CLIENTS) $(FILES)

all:	$(GOAL)

rusers:	rusers.o
	$(CC) $(CFLAGS) -o $@ rusers.o $(LINKLIBS) $(LDFLAGS) $(LDLIBS)

rpc.rusersd: rpc.rusersd.o	
	$(CC) $(CFLAGS) -o $@ rpc.rusersd.o $(LINKLIBS) $(LDFLAGS) $(LDLIBS)

spray:	$(SCLNTOBJS)
	$(CC) $(CFLAGS) -o $@ $(SCLNTOBJS) $(LINKLIBS) $(LDFLAGS) $(LDLIBS)

rpc.sprayd: $(SSVCOBJS)
	$(CC) $(CFLAGS) -o $@ $(SSVCOBJS) $(LINKLIBS) $(LDFLAGS) $(LDLIBS)

rwall: $(WCLNTOBJS)
	$(CC) $(CFLAGS) -o $@ $(WCLNTOBJS) $(LINKLIBS) $(LDFLAGS) $(LDLIBS)

rpc.rwalld: $(WSVCOBJS)
	$(CC) $(CFLAGS) -o $@ $(WSVCOBJS) $(LINKLIBS) $(LDFLAGS) $(LDLIBS)
$(DIRS):
	mkdir $@
	$(CH)chmod 755 $@
	$(CH)chgrp sys $@
	$(CH)chown root $@

install: $(GOAL) $(DIRS)
	$(INS) -f $(RWALL) -m 0555 -u bin -g bin rpc.rwalld
	$(INS) -f $(RUSERS) -m 0555 -u bin -g bin rpc.rusersd
	$(INS) -f $(SPRAY) -m 0555 -u bin -g bin rpc.sprayd
	$(INS) -f $(DESTCLIENTS) -m 0555 -u bin -g bin rusers
	$(INS) -f $(DESTSERVERS) -m 0555 -u bin -g bin rwall
	$(INS) -f $(DESTSERVERS) -m 0555 -u bin -g bin spray
	$(INS) -f $(DESTSERVERS) -m 0555 -u bin -g bin keyprop
	for i in $(FILES) ; \
	do \
		$(INS) -f $(TICLTS) -m 0444 -u root -g sys $$i ; \
		$(INS) -f $(TICOTS) -m 0444 -u root -g sys $$i ; \
		$(INS) -f $(TICOTSORD) -m 0444 -u root -g sys $$i ; \
	done 
	for i in $(ETCFILES) ; \
	do \
		$(INS) -f $(ETC) -m 0644 -u root -g sys $$i ; \
	done

lint:
	$(LINT) $(CPPFLAGS) $(SRCS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(CLIENTS) $(SERVERS)
