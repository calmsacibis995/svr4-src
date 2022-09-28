#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksetup/except/Makefile	1.3.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
EXCDIR = $(OAMBASE)/menu/backup_service/setup/exception_list
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=add display remove
MENU=except.menu

except: all

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

install: all $(EXCDIR)
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(EXCDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(EXCDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(EXCDIR):
	if [ ! -d `dirname $(EXCDIR)` ] ;\
	then \
		mkdir -p `dirname $(EXCDIR)` ;\
	fi
	if [ ! -d $(EXCDIR) ] ;\
	then \
		mkdir -p $(EXCDIR) ;\
	fi
