#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkstat/select/Makefile	1.5.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
SELDIR = $(OAMBASE)/menu/backup_service/status/selective
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = select

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.select Menu.states Text.select Text.errs Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(SELDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(SELDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(SELDIR):
	if [ ! -d `dirname $(SELDIR)` ] ;\
	then \
		mkdir -p `dirname $(SELDIR)` ;\
	fi
	if [ ! -d $(SELDIR) ] ;\
	then \
		mkdir -p $(SELDIR) ;\
	fi
