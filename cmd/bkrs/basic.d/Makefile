#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:basic.d/Makefile	1.1.3.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BOPERDIR = $(OAMBASE)/menu/backup_service/backup/basic
INSTALL = install
PRODUCT = basic

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Help Menu.backup Text.history Text.privil

P_FILES=Form.files Menu.backsel Menu.backusr Menu.private
SY_FILES=Form.files Menu.backsel Menu.backsys Menu.system Menu.backusr Menu.users
SC_FILES=Form.add Form.chg Menu.chg Menu.day Menu.del Menu.hour Menu.minute Menu.month Menu.sched Menu.week Text.addcfm Text.already Text.chgcfm Text.delcfm Text.display Text.nocron Text.nodel Text.nosched Text.notape Text.privil

all: $(O_SHFILES)

clean:

clobber:

install: all $(BOPERDIR) BOPERDIRSUB
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR) $$i ;\
	done

	for i in $(P_FILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR)/private private/$$i ;\
	done

	for i in $(SY_FILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR)/system system/$$i ;\
	done

	for i in $(SC_FILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR)/sched sched/$$i ;\
	done


$(BOPERDIR):
	if [ ! -d `dirname $(BOPERDIR)` ] ;\
	then \
		mkdir -p `dirname $(BOPERDIR)` ;\
	fi
	if [ ! -d $(BOPERDIR) ] ;\
	then \
		mkdir -p $(BOPERDIR) ;\
	fi
BOPERDIRSUB:
	if [ ! -d `dirname $(BOPERDIR)/private` ] ;\
	then \
		mkdir -p `dirname $(BOPERDIR)/private` ;\
	fi
	if [ ! -d $(BOPERDIR)/private ] ;\
	then \
		mkdir -p $(BOPERDIR)/private ;\
	fi

	if [ ! -d `dirname $(BOPERDIR)/system` ] ;\
	then \
		mkdir -p `dirname $(BOPERDIR)/system` ;\
	fi
	if [ ! -d $(BOPERDIR)/system ] ;\
	then \
		mkdir -p $(BOPERDIR)/system ;\
	fi

	if [ ! -d `dirname $(BOPERDIR)/sched` ] ;\
	then \
		mkdir -p `dirname $(BOPERDIR)/sched` ;\
	fi
	if [ ! -d $(BOPERDIR)/sched ] ;\
	then \
		mkdir -p $(BOPERDIR)/sched ;\
	fi
