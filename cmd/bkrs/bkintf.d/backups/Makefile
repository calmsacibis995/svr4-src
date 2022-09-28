#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/Makefile	1.3.4.1"

INC = $(ROOT)/usr/include
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
BACKUPDIR = $(OAMBASE)/menu/backup_service
SADMDIR=$(ROOT)/var/sadm
PKGDIR=$(SADMDIR)/pkg/oam
MIDIR = $(PKGDIR)/save/intf_install
MIFILE = bkup.mi
HELPFILE = Help
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=backup bkhist bkoper bkrem bksched bksetup bkstat
MENUB = backup.menu
MENUE = ext.menu

backups: all

all: $(PRODUCTS) $(MIFILE) $(HELPFILE) $(MENUB) $(MENUE)

.sh:
	cp $< $*

$(PRODUCTS):
	cd $(@); $(MAKE) -$(MAKEFLAGS) $(@); cd ..

$(MIFILE):
	grep -v "^#ident" 08455426588.mi >$(MIFILE)

clean:

clobber:
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done
	rm -f $(MIFILE)

install: all $(BACKUPDIR) $(PKGDIR) $(MIDIR)
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(MIDIR) $(MIFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BACKUPDIR) $(HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BACKUPDIR) $(MENUB)
	$(INSTALL) -m 644 -g bin -u bin -f $(BACKUPDIR) $(MENUE)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BACKUPDIR):
	- if [ ! -d `dirname $(BACKUPDIR)` ] ;\
	then \
		mkdir -p $(OAMBASE)/menu ;\
		mkdir -p $(OAMBASE)/menu/backup_service ;\
	fi
$(PKGDIR):
	- if [ ! -d `dirname $(PKGDIR)` ] ;\
	then \
		mkdir -p $(SADMDIR) ;\
		mkdir -p $(SADMDIR)/pkg ;\
		mkdir -p $(SADMDIR)/pkg/oam ;\
	fi
$(MIDIR):
	- if [ ! -d `dirname $(MIDIR)` ] ;\
	then \
		mkdir -p $(SADMDIR)/pkg/oam/save ;\
		mkdir -p $(SADMDIR)/pkg/oam/save/intf_install ;\
	fi
