#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkhist/Makefile	1.7.3.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BHISTDIR = $(OAMBASE)/menu/backup_service/history
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=full limit select
MENU = bkhist.menu

bkhist: all

O_SHFILES =

O_HELPFILE = Help

all: $(O_SHFILES) $(O_HELPFILE) $(PRODUCTS) $(MENU)

.sh:
	cp $< $*

$(PRODUCTS):
	cd $(@); $(MAKE) -$(MAKEFLAGS) $(@); cd ..

clean:

clobber:
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done

install: all $(BHISTDIR)
	for f in $(PRODUCTS) ;\
	do \
		cd $$f ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ;\
		cd .. ;\
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(BHISTDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BHISTDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BHISTDIR):
	if [ ! -d `dirname $(BHISTDIR)` ] ;\
	then \
		mkdir -p `dirname $(BHISTDIR)` ;\
	fi
	if [ ! -d $(BHISTDIR) ] ;\
	then \
		mkdir -p $(BHISTDIR) ;\
	fi
