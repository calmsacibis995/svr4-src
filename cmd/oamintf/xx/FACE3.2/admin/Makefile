#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:xx/FACE3.2/admin/Makefile	1.1"

SABIN1=$(ROOT)/usr/sysadm/bin
SABIN2=$(ROOT)/usr/sadm/sysadm/bin
SABIN3=$(ROOT)/usr/sadm/sysadm/admsets
ETCDIR=$(ROOT)/etc
UBIN=$(ROOT)/usr/bin
USBIN=$(ROOT)/usr/sbin

BINARY=menu_add
PRODUCTS=$(BINARY) .useradm base-adm

INSTALL = install

all: $(PRODUCTS)
	
$(BINARY):
	@echo "***** Making FACE admin files/cmds"
	$(CC) -s -O $(@).c -o $(@)

.useradm:
	grep -v "^#ident	" useradm > .useradm

base-adm:
	grep -v "^#ident	" Base-adm > base-adm

install: all
	-[ -d $(SABIN1) ] || mkdir -p $(SABIN1)
	-[ -d $(SABIN2) ] || mkdir -p $(SABIN2)
	-[ -d $(SABIN3) ] || mkdir -p $(SABIN3)
	-[ -d $(ETCDIR) ] || mkdir -p $(ETCDIR)
	-[ -d $(UBIN) ] || mkdir -p $(UBIN)
	-[ -d $(USBIN) ] || mkdir -p $(USBIN)
	$(INSTALL) -m 755 -g bin -u bin -f $(SABIN1) base-adm
	$(INSTALL) -m 755 -g bin -u bin -f $(ETCDIR) .useradm
	$(INSTALL) -m 755 -g bin -u bin -f $(UBIN) menu_add
	-ln $(SABIN1)/base-adm $(SABIN2)/base-adm
	-ln $(SABIN1)/base-adm $(SABIN3)/base-adm
	-ln $(UBIN)/menu_add $(UBIN)/menu_del
	-ln $(UBIN)/menu_add $(USBIN)/menu_add
	-ln $(UBIN)/menu_add $(USBIN)/menu_del

clean:
	@echo "***** Cleaning FACE admin files/cmds"
	rm -f *.o dummyfile

clobber: clean
	@echo "***** Clobbering FACE admin files/cmds"
	rm -f $(PRODUCTS)
