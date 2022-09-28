#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/OBJECTS/hwset/Makefile	1.1"

MENUS=$(ROOT)/usr/vmsys/OBJECTS/hwset
MENUFILES=Menu.hard Text.compat

INSTALL = install

all:
	@echo "nothing to build in OBJECTS/hwset."

install: all
	[ -d $(MENUS) ] || mkdir -p $(MENUS)
	-for f in $(MENUFILES) ; \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(MENUS) $$f ; \
	done

clean:
	@echo "nothing to clean in OBJECTS/hwset."

clobber: clean
	@echo "nothing to clobber in OBJECTS/hwset."
