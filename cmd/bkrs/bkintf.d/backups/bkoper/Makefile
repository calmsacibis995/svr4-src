#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bkoper/Makefile	1.7.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BOPERDIR = $(OAMBASE)/menu/backup_service/respond
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = bkoper

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.bkoper Text.bjpok Text.bjpnok Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(BOPERDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(BOPERDIR):
	if [ ! -d `dirname $(BOPERDIR)` ] ;\
	then \
		mkdir -p `dirname $(BOPERDIR)` ;\
	fi
	if [ ! -d $(BOPERDIR) ] ;\
	then \
		mkdir -p $(BOPERDIR) ;\
	fi
