#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/HELP/restore/Makefile	1.1"

HMENUS=$(ROOT)/usr/vmsys/HELP/restore
HMENUDIRS=system
HMENUFILES=T.hdevice T.hoverwr1 T.hprivate T.hrestore T.hselect

INSTALL = install

all:
	@echo "nothing to build in VMSYS/HELP."

install: all
	-for d in $(HMENUDIRS); \
	do \
		mkdir -p $(HMENUS)/$$d; \
		cd $$d; \
		for x in *; \
		do \
			$(INSTALL) -m 644 -g bin -u bin -f $(HMENUS)/$$d $$x ; \
		done; \
		cd ..; \
	done; \
	for d in $(HMENUFILES); \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(HMENUS) $$d; \
	done;

clean:

clean:
	@echo "nothing to clean in VMSYS/HELP."

clobber: clean
	@echo "nothing to clobber in VMSYS/HELP."
