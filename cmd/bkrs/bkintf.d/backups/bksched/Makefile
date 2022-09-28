#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksched/Makefile	1.7.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BKSCHEDDIR = $(OAMBASE)/menu/backup_service/schedule
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=add display modify remove
MENU = bksched.menu

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

install: all $(BKSCHEDDIR)
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(BKSCHEDDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BKSCHEDDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BKSCHEDDIR):
	if [ ! -d `dirname $(BKSCHEDDIR)` ] ;\
	then \
		mkdir -p `dirname $(BKSCHEDDIR)` ;\
	fi
	if [ ! -d $(BKSCHEDDIR) ] ;\
	then \
		mkdir -p $(BKSCHEDDIR) ;\
	fi
