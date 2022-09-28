#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/OBJECTS/dskopts/Makefile	1.1"

MENUS=$(ROOT)/usr/vmsys/OBJECTS/dskopts
MENUFILES=Menu.dskopts

INSTALL = install

all:
	@echo "nothing to build in OBJECTS/dskopts."

install: all
	[ -d $(MENUS) ] || mkdir -p $(MENUS)
	-for f in $(MENUFILES) ; \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(MENUS) $$f ; \
	done

clean:
	@echo "nothing to clean in OBJECTS/dskopts."

clobber: clean
	@echo "nothing to clobber in OBJECTS/dskopts."
