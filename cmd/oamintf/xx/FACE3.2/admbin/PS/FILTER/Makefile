#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/Makefile	1.1"

BINDIR=$(ROOT)/usr/vmsys/admin/PS/FILTER
BINARY=chgflt
SHELLS=add chkdel chkmode delete mitem mode name option pname ptype seek
PRODUCTS=$(BINARY) $(SHELLS)

INSTALL = install

all: $(PRODUCTS)
	@echo "***** Making VMSYS/admin/PS/FILTER"

$(BINARY):
	$(CC) -s -O $(@).c -o $(@)

$(SHELLS):
	-for f in $(SHELLS) ; \
	do \
		cp $$f.sh $$f ; \
	done

install: all
	-[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	-for f in $(PRODUCTS) ; \
	do \
		$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) $$f ; \
	done

clean:
	@echo "***** Cleaning VMSYS/admin/PS/FILTER"
	rm -f *.o dummyfile

clobber: clean
	@echo "***** Clobbering VMSYS/admin/PS/FILTER"
	rm -f $(PRODUCTS) dummyfile
