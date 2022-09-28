#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/HELP/hwset/Makefile	1.1"

HMENUS=$(ROOT)/usr/vmsys/HELP/hwset
HMENUDIRS=
HMENUFILES=T.hhard

INSTALL = install

all:
	@echo "nothing to build in VMSYS/HELP/hwset."

install: all
	[ -d $(HMENUS) ] || mkdir -p $(HMENUS)
	-for f in $(HMENUFILES) ; \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(HMENUS) $$f ; \
	done

clean:
	@echo "nothing to clean in VMSYS/HELP/hwset."

clobber: clean
	@echo "nothing to clobber in VMSYS/HELP/hwset."
