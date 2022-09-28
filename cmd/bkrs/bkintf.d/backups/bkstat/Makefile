#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkstat/Makefile	1.1.3.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BKSTATDIR = $(OAMBASE)/menu/backup_service/status
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=full limit modify select
MENU = bkstat.menu

rsstat: all

.sh:
	cp $< $*

O_HELPFILE = Help

all: $(O_HELPFILE) $(PRODUCTS) $(MENU)

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

install: all $(BKSTATDIR)
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(BKSTATDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(BKSTATDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BKSTATDIR):
	if [ ! -d `dirname $(BKSTATDIR)` ] ;\
	then \
		mkdir -p `dirname $(BKSTATDIR)` ;\
	fi
	if [ ! -d $(BKSTATDIR) ] ;\
	then \
		mkdir -p $(BKSTATDIR) ;\
	fi
