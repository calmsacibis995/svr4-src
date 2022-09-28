#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkrem/add/Makefile	1.4.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
ADDDIR = $(OAMBASE)/menu/backup_service/reminder/add
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = add

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.add Text.addok Text.addnok Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(ADDDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(ADDDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(ADDDIR):
	if [ ! -d `dirname $(ADDDIR)` ] ;\
	then \
		mkdir -p `dirname $(ADDDIR)` ;\
	fi
	if [ ! -d $(ADDDIR) ] ;\
	then \
		mkdir -p $(ADDDIR) ;\
	fi
