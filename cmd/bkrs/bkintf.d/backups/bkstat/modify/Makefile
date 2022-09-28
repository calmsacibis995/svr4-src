#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkstat/modify/Makefile	1.4.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
CHGDIR = $(OAMBASE)/menu/backup_service/status/modify
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = modify

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.modify Form.control Text.modok Text.modnok Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(CHGDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(CHGDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(CHGDIR):
	if [ ! -d `dirname $(CHGDIR)` ] ;\
	then \
		mkdir -p `dirname $(CHGDIR)` ;\
	fi
	if [ ! -d $(CHGDIR) ] ;\
	then \
		mkdir -p $(CHGDIR) ;\
	fi
