#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/HELP/backup/Makefile	1.1"

HMENUS=$(ROOT)/usr/vmsys/HELP/backup
HMENUDIRS=private sched system
HMENUFILES=T.hbackup

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
	$(INSTALL) -m 644 -g bin -u bin -f $(HMENUS) $(HMENUFILES)

clean:

clean:
	@echo "nothing to clean in VMSYS/HELP."

clobber: clean
	@echo "nothing to clobber in VMSYS/HELP."
