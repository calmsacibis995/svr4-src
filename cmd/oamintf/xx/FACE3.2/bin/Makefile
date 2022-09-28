#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/bin/Makefile	1.1"

# Note: In 3.2 FACE, some shells install as .sh and others do not!

FACEBIN=$(ROOT)/usr/vmsys/bin
BINARY=getchg getdel getword slash
SHELLS=doexec getname
PRODUCTS=$(BINARY) $(SHELLS)

INSTALL = install

all: $(PRODUCTS)
	
$(BINARY):
	@echo "***** Making VMSYS/bin"
	$(CC) -s -O $(@).c -o $(@)

$(SHELLS):
	-for f in $(SHELLS) ; \
	do \
		cp $$f.sh $$f ; \
	done

install: all
	-[ -d $(FACEBIN) ] || mkdir -p $(FACEBIN)

	-for f in $(PRODUCTS) ; \
	do \
		$(INSTALL) -m 755 -g bin -u bin -f $(FACEBIN) $$f ; \
	done

clean:
	@echo "***** Cleaning VMSYS/bin"
	rm -f *.o dummyfile

clobber: clean
	@echo "***** Clobbering VMSYS/bin"
	rm -f $(PRODUCTS)
