#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkrem/Makefile	1.3.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BKMSGDIR = $(OAMBASE)/menu/backup_service/reminder
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=add display modify remove
MENU = bkrem.menu

rsstat: all

.sh:
	cp $< $*

O_HELPFILE = Help

all: $(O_SHFILES) $(O_HELPFILE) $(PRODUCTS) $(MENU)

$(PRODUCTS):
	cd $(@); $(MAKE) -$(MAKEFLAGS) $(@); cd ..

clean:

clobber:
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

install: all $(BKMSGDIR)
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(BKMSGDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BKMSGDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BKMSGDIR):
	if [ ! -d `dirname $(BKMSGDIR)` ] ;\
	then \
		mkdir -p `dirname $(BKMSGDIR)` ;\
	fi
	if [ ! -d $(BKMSGDIR) ] ;\
	then \
		mkdir -p $(BKMSGDIR) ;\
	fi
