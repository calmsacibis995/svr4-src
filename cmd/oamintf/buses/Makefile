#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:buses/Makefile	1.1.1.1"

DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
OAMBASE=/usr/sadm/sysadm
BUSDIR = $(ROOT)$(OAMBASE)/menu/buses
HELPSRCDIR = ./HELP
INSTALL = install
STRIP = strip
SIZE = size

.sh:
	cp $< $*

O_SHFILES =

O_DFILES = buses.menu

O_HELPFILES = Help

all: $(O_SHFILES) $(O_HELPFILES)

$(O_HELPFILES):
	grep -v "^#ident" $(HELPSRCDIR)/id.buses.help >$(@)

clean:

clobber: clean
	rm -f $(O_SHFILES) dummyfile

	for i in $(O_HELPFILES) ;\
	do \
		rm -f $$i ;\
	done

install: all $(BUSDIR)
	for i in $(O_DFILES) $(O_HELPFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BUSDIR) $$i ;\
	done

size: all
	$(SIZE)

strip: all
	$(STRIP)

$(BUSDIR):
	-if [ ! -d $(BUSDIR) ] ;\
	then \
		mkdir -p $(BUSDIR) ;\
	fi
