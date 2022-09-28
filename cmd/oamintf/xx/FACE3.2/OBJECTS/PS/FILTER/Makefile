#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/OBJECTS/PS/FILTER/Makefile	1.1"

MENUS=$(ROOT)/usr/vmsys/OBJECTS/PS/FILTER
MENUFILES=Form.add Form.cd Form.change Form.fadd Menu.del Menu.filter \
	  Menu.input Menu.output Menu.pname Menu.ptype \
	  Text.addcfm Text.all Text.chgcfm Text.delcfm Text.display \
	  Text.nochg Text.nofltr Text.none

INSTALL = install

all:
	@echo "nothing to build in OBJECTS/PS/FILTER."

install: all
	-[ -d $(MENUS) ] || mkdir -p $(MENUS)
	-for f in $(MENUFILES) ; \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(MENUS) $$f ; \
	done

clean:
	@echo "nothing to clean in OBJECTS/PS/FILTER."

clobber: clean
	@echo "nothing to clobber in OBJECTS/PS/FILTER."
