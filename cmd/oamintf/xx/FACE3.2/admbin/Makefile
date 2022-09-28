#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/admbin/Makefile	1.1"

# Note: In 3.2 FACE, some shells install as .sh and others do not!

BINDIR=$(ROOT)/usr/vmsys/admin
OAMBIN=$(ROOT)/usr/sadm/sysadm/bin
DOTFILE=chkuser
OTHBINARY=chkuser
BINARY=
SHELLS=
PRODUCTS=$(BINARY) $(SHELLS)
SUBDIR=PS

INSTALL = install

all: $(PRODUCTS) $(OTHBINARY)

$(OTHBINARY):
	@echo "***** Making VMSYS/admin"
	$(CC) -s -O $(@).c -o $(@)

install: all
	-[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	-[ -d $(OAMBIN) ] || mkdir -p $(OAMBIN)

	-for f in $(DOTFILE) ; \
	do \
		cp $$f .$$f ; \
	done

	$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) .chkuser
	-ln $(BINDIR)/.chkuser $(OAMBIN)/.chkuser

	-for d in $(SUBDIR) ; \
	do \
		( cd $$d ; $(MAKE) $(@) ) ; \
	done

clean clobber:
	@echo "***** Cleaning/Clobbering VMSYS/admin"
	rm -f *.o $(PRODUCTS) $(OTHBINARY) .$(DOTFILE) dummyfile
	-for d in $(SUBDIR) ; \
	do \
		( cd $$d ; $(MAKE) $(@) ) ; \
	done

