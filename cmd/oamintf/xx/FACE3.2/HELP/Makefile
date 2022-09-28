#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/HELP/Makefile	1.1"

HMENUS=$(ROOT)/usr/vmsys/HELP
HMENUDIRS=PS backup dskopts hwset logins mailset restore sched
HMENUFILES=T.h1.help T.h7.cmd T.hadmin

INSTALL = install

all:
	@echo "nothing to build in VMSYS/HELP."

install: all
	-[ -d $(HMENUS) ] || mkdir -p $(HMENUS)
	-for f in $(HMENUFILES) ; \
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(HMENUS) $$f ; \
	done

	-for dir in $(HMENUDIRS) ; \
	do \
		( cd $$dir ; $(MAKE) $@ ) ; \
	done

clean clobber:
	-for dir in $(HMENUDIRS) ; \
	do \
		( cd $$dir ; $(MAKE) $@ ) ; \
	done

