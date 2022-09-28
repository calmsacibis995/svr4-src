#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsintf.d/restores/restore/Makefile	1.6.2.1"
OAMBASE=$(ROOT)/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
RESTDIR = $(OAMBASE)/menu/restores/restore
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = restore

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.restore Text.rest Text.err Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(RESTDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(RESTDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(RESTDIR):
	if [ ! -d `dirname $(RESTDIR)` ] ;\
	then \
		mkdir -p `dirname $(RESTDIR)` ;\
	fi
	if [ ! -d $(RESTDIR) ] ;\
	then \
		mkdir -p $(RESTDIR) ;\
	fi
