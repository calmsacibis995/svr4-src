#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsintf.d/restores/Makefile	1.1.3.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
RESTOREDIR = $(OAMBASE)/menu/restores
SADMDIR=$(ROOT)/var/sadm
PKGDIR=$(SADMDIR)/pkg/oam
MIDIR = $(PKGDIR)/save/intf_install
MIFILE = rest.mi
HELPFILE = Help
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=restore rsstat setup rsoper
MENUB = restore.menu
MENUE = ext.menu

restores: all

all: $(PRODUCTS) $(MIFILE) $(HELPFILE)

.sh:
	cp $< $*

$(PRODUCTS):
	cd $(@); $(MAKE) -$(MAKEFLAGS) $(@); cd ..

$(MIFILE):
	grep -v "^#ident" 16122026388.mi >$(MIFILE)

clean:

clobber:
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done
	rm -f $(MIFILE)

install: all $(RESTOREDIR) $(PKGDIR) $(MIDIR)
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(MIDIR) $(MIFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(RESTOREDIR) $(HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(RESTOREDIR) $(MENUB)
	$(INSTALL) -m 644 -g bin -u bin -f $(RESTOREDIR) $(MENUE)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(RESTOREDIR):
	if [ ! -d `dirname $(RESTOREDIR)` ] ;\
	then \
		mkdir -p `dirname $(RESTOREDIR)` ;\
	fi
	if [ ! -d $(RESTOREDIR) ] ;\
	then \
		mkdir -p $(RESTOREDIR) ;\
	fi
$(PKGDIR):
	if [ ! -d `dirname $(PKGDIR)` ] ;\
	then \
		mkdir -p `dirname $(PKGDIR)` ;\
	fi
	if [ ! -d $(PKGDIR) ] ;\
	then \
		mkdir -p $(PKGDIR) ;\
	fi
$(MIDIR):
	if [ ! -d `dirname $(MIDIR)` ] ;\
	then \
		mkdir -p `dirname $(MIDIR)` ;\
	fi
	if [ ! -d $(MIDIR) ] ;\
	then \
		mkdir -p $(MIDIR) ;\
	fi
