#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rbasic.d/Makefile	1.1.3.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
BOPERDIR = $(OAMBASE)/menu/restores/restore/basic
INSTALL = install
PRODUCT = rbasic

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Help Menu.restore Text.privil

P_FILES=Form.overwr Form.overwr2 Menu.device Menu.device2 Menu.private Menu.select
S_FILES=Form.overwr Form.overwr2 Menu.device Menu.device2 Menu.system Menu.select
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
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR)/private  private/$$i ;\
	done

	for i in $(S_FILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(BOPERDIR)/system system/$$i ;\
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
