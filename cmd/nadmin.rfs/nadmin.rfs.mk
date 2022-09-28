#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nadmin.rfs:nadmin.rfs.mk	1.5.3.1"
#makefile for rfs administration screens

OAMBASE=/usr/sadm/sysadm
TARGETDIR = $(ROOT)$(OAMBASE)/menu/netservices/remote_files
MDIR = ../nadmin.rfs.mk
INS = install

install:
	for i in * ; do \
		if [ -d $$i ] ; then \
		if [ ! -d $(TARGETDIR)/$$i ] ; then \
			mkdir -p $(TARGETDIR)/$$i  ;\
		fi ;\
			cd $$i ;\
			$(MAKE) "TARGETDIR=$(TARGETDIR)/$$i" "MDIR=../$(MDIR)" -f $(MDIR);\
			cd .. ;\
		else if [ $$i != "nadmin.rfs.mk" ] ; then \
			if [ $$i = "system.mk" ] ; then \
				$(MAKE) "TARGETDIR=$(TARGETDIR)" -f $$i ;\
				$(INS) -m 644 -g bin -u bin -f $(TARGETDIR)  getaddr ;\
				$(MAKE)  "TARGETDIR=$(TARGETDIR)" -f $$i clobber ;\
			else if [ $$i != "getaddr.c" ] ; then\
				echo "installing $$i" ;\
				$(INS) -m 644 -g bin -u bin -f $(TARGETDIR)  $$i ;\
			     fi ;\
			fi ;\
		     fi ;\
		fi  ;\
	done

clobber:
