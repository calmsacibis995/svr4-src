#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/OAM4.0/Makefile	1.1"

INSTALL  = install
INTF_INSTALL = $(ROOT)/var/sadm/pkg/oam/save/intf_install
OAMBASE  = $(ROOT)/usr/sadm/sysadm

all: .colorpref

.colorpref:
	@echo "====== Making MI and colorpref files"
	grep -v "^#ident	" colorpref > .colorpref

install: all
	[ -d $(OAMBASE)/pref ] || mkdir -p $(OAMBASE)/pref
	[ -d $(INTF_INSTALL) ] || mkdir -p $(INTF_INSTALL)
	$(INSTALL) -m 644 -g bin -u bin -f $(OAMBASE)/pref .colorpref
	$(INSTALL) -m 644 -g bin -u bin -f $(INTF_INSTALL) oam.mi
	$(INSTALL) -m 644 -g bin -u bin -f $(INTF_INSTALL) file.mi
	$(INSTALL) -m 644 -g bin -u bin -f $(INTF_INSTALL) devices.mi

clean:
	@echo "nothing to clean"

clobber: clean
	@echo "====== Clobbering MI and colorpref files"
	rm -f .colorpref
