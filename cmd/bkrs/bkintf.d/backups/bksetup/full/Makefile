#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksetup/full/Makefile	1.3.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
FULLDIR = $(OAMBASE)/menu/backup_service/setup/full
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = full

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.full Text.full Text.errs Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(FULLDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(FULLDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(FULLDIR):
	if [ ! -d `dirname $(FULLDIR)` ] ;\
	then \
		mkdir -p `dirname $(FULLDIR)` ;\
	fi
	if [ ! -d $(FULLDIR) ] ;\
	then \
		mkdir -p $(FULLDIR) ;\
	fi
