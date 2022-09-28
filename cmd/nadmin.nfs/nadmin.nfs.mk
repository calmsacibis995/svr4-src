#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nadmin.nfs:nadmin.nfs.mk	1.4"
#makefile for nfs administration screens

OAMBASE=/usr/sadm/sysadm
TARGETDIR = $(ROOT)$(OAMBASE)/menu/netservices/remote_files
MDIR = ../nadmin.nfs.mk
INS = install

install:
	for i in * ; do \
		if [ -d $$i ] ; then \
			if [ ! -d $(TARGETDIR)/$$i ] ; then \
			mkdir -p $(TARGETDIR)/$$i  ;\
			fi ; \
			cd $$i ;\
			make  -i "TARGETDIR=$(TARGETDIR)/$$i" "MDIR=../$(MDIR)" -f $(MDIR);\
			cd .. ;\
		else \
			if [ $$i != "nadmin.nfs.mk" ] ;\
			then \
				echo "installing $$i" ;\
				$(INS) -m 644 -g bin -u bin -f $(TARGETDIR)  $$i ;\
			fi ;\
		fi  ;\
	done

clean:

clobber: clean
