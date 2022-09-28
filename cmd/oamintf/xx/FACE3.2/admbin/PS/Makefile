#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/Makefile	1.1"

BINDIR=$(ROOT)/usr/vmsys/admin/PS
SUBDIR=CONFIG FILTER FORM WHEEL
BINARY=
SHELLS=
PRODUCTS=$(BINARY) $(SHELLS)

INSTALL = install

all:
	@echo "***** Making VMSYS/admin/PS"
	@echo "nothing to build."
	
install: all
	[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	-for f in $(SUBDIR) ; \
	do \
		( cd $$f ; $(MAKE) $(@) ) ; \
	done

clean clobber:
	-for f in $(SUBDIR) ; \
	do \
		( cd $$f ; $(MAKE) $(@) ) ; \
	done
