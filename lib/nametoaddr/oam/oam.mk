#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nametoaddr:oam/oam.mk	1.6.2.2"
#makefile for name to address administration screens

OAMBASE=/usr/sadm/sysadm
TARGETDIR = $(ROOT)$(OAMBASE)/menu/netservices
MDIR = ../oam.mk
INSTALL = install

install:
	-for i in * ; do \
		if [ -d $$i ] ; then \
			if [ ! -d $(TARGETDIR)/$$i ] ; then \
				mkdir -p $(TARGETDIR)/$$i  ;\
			fi ; \
			cd $$i ;\
			make  -i "TARGETDIR=$(TARGETDIR)/$$i" "MDIR=../$(MDIR)" -f $(MDIR);\
			cd .. ;\
		else \
			if [ $$i != "oam.mk" ] ;\
			then \
			if [ `basename $(TARGETDIR)` = "HELP" ] ; then \
				grep -v \"^#ident\" $(TARGETDIR)/$$i >/usr/tmp/$$i;\
				mv /usr/tmp/$$i $(TARGETDIR)/$$i;\
				echo "installing $$i";\
				$(INSTALL) -m 644 -g bin -u bin -f $(TARGETDIR) $$i;\
			else \
				echo "installing $$i" ;\
				$(INSTALL) -m 755 -g bin -u bin -f $(TARGETDIR)  $$i ;\
			fi ;\
			fi ;\
		fi  ;\
	done
			
clobber:
