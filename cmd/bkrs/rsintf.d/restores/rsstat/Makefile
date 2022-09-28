#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsintf.d/restores/rsstat/Makefile	1.9.2.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
RSSTATDIR = $(OAMBASE)/menu/restores/status
INSTALL = install
STRIP = strip
SIZE = size
PRODUCTS=full modify select
MENU=rsstat.menu

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

install: all $(RSSTATDIR)
	for f in $(PRODUCTS) ; \
	do \
		cd $$f; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

	$(INSTALL) -m 644 -g bin -u bin -f $(RSSTATDIR) $(O_HELPFILE)
	$(INSTALL) -m 644 -g bin -u bin -f $(RSSTATDIR) $(MENU)

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(RSSTATDIR):
	if [ ! -d `dirname $(RSSTATDIR)` ] ;\
	then \
		mkdir -p `dirname $(RSSTATDIR)` ;\
	fi
	if [ ! -d $(RSSTATDIR) ] ;\
	then \
		mkdir -p $(RSSTATDIR) ;\
	fi
