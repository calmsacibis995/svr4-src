#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:port_quick/quick.mk	1.1.2.1"

OAMBASE=/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)$(OAMBASE)/menu/ports/port_quick
BINDIR = $(ROOT)$(OAMBASE)/bin
INSTALL = install
LINK = ln

SHFILES = q-add q-rm

CFILES = isastream

O_DFILES = Form.add Form.rm Menu.ap Menu.rp Text.cfa Text.cfr Text.priv quick.menu Help

all: isastream shells

isastream:
		$(CC) -O -c isastream.c
		$(CC) -O -s -o isastream isastream.o $(LDLIBS)
shells:
		cp q-add.sh q-add
		cp q-rm.sh q-rm

clean:
	-rm -f *.o $(SHFILES) $(CFILES)

clobber: clean
	-rm -f $(SHFILES) $(CFILES)

install: all $(INSDIR) $(TASKS)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR) $$i ;\
	done

	for i in $(SHFILES) ;\
	do \
		$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) $$i ;\
	done

	for i in $(CFILES) ;\
	do \
		$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) $$i ;\
	done

size: all

strip: all

$(INSDIR):
	if [ ! -d $(INSDIR) ] ;\
	then \
		mkdir -p $(INSDIR) ;\
	fi
