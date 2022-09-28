#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/backup/Makefile	1.7.3.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BKUPDIR = $(OAMBASE)/menu/backup_service/backup
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = backup

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.backup Text.bkpok Text.bkpnok Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(BKUPDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BKUPDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(BKUPDIR):
	if [ ! -d `dirname $(BKUPDIR)` ] ;\
	then \
		mkdir -p `dirname $(BKUPDIR)` ;\
	fi
	if [ ! -d $(BKUPDIR) ] ;\
	then \
		mkdir -p $(BKUPDIR) ;\
	fi
