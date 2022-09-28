#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp.admin:lp.admin.mk	1.13.5.1"

#
#	Makefile for lp.admin: OAM interface for lp
#

OWNER=root
GRP=sys

PKG=$(ROOT)/var/sadm/pkg
PKGINST=lp
PKGLP=$(PKG)/$(PKGINST)
PKGSAV=$(PKGLP)/save
PKGMI=$(PKGSAV)/intf_install

OAMBASE=$(ROOT)/usr/sadm/sysadm
INTFBASE=$(OAMBASE)/add-ons/lp
INS=install
DIRS=\
	$(PKG) \
	$(PKGLP) \
	$(PKGSAV) \
	$(PKGMI) \
	$(INTFBASE) \
	$(INTFBASE)/printers \
	$(INTFBASE)/printers/classes \
	$(INTFBASE)/printers/filters \
	$(INTFBASE)/printers/forms \
	$(INTFBASE)/printers/operations \
	$(INTFBASE)/printers/printers \
	$(INTFBASE)/printers/printers/add \
	$(INTFBASE)/printers/printers/modify \
	$(INTFBASE)/printers/printers/terminfo \
	$(INTFBASE)/printers/priorities \
	$(INTFBASE)/printers/reports \
	$(INTFBASE)/printers/requests \
	$(INTFBASE)/printers/systems

install: $(DIRS)
	#
	$(INS) -f $(PKGMI) -m 0644 -u $(OWNER) -g $(GRP) lp.mi
	#
	find printers -type f -print | \
	sed 's,^\(.*\)/,\1 ,p' | \
	while read dir file ;\
	do \
		$(INS) -f $(INTFBASE)/$$dir -m 0644 -u $(OWNER) -g $(GRP) $$dir/$$file || exit 1 ;\
	done
	chmod 755 $(INTFBASE)/printers/printers/add/*.sh
	chmod 755 $(INTFBASE)/printers/*/*.sh

$(DIRS):
	mkdir -p $@
	$(CH)-chmod 0755 $@
	$(CH)-chgrp $(GRP) $@
	$(CH)-chown $(OWNER) $@

clobber:
