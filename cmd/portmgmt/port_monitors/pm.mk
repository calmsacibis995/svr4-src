#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:port_monitors/pm.mk	1.5.2.1"

OAMBASE=/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)$(OAMBASE)/menu/ports/port_monitors
BINDIR = $(ROOT)$(OAMBASE)/bin
INSTALL = install
LINK = ln

TASKS = add disable enable list modify remove start stop

O_DFILES = pm.menu Help

add=Form.pm_add 

disable=Menu.pm_disabl

enable=Menu.pm_enable

list=Menu.pm_list Menu.pm_lstag Menu.pm_lstype Text.pm_list Text.pm_lsall

modify=Menu.pm_modify Form.pm_modify

remove=Menu.pm_remove

start=Menu.pm_start

stop=Menu.pm_stop

all: 

clean:

clobber: clean
	
install: all $(INSDIR) $(TASKS)
# add
	for i in $(add) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/add $$i ;\
	done

# disable
	for i in $(disable) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/disable $$i ;\
	done

# enable
	for i in $(enable) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/enable $$i ;\
	done

# list
	for i in $(list) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/list $$i ;\
	done

# modify
	for i in $(modify) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/modify $$i ;\
	done

# remove
	for i in $(remove) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/remove $$i ;\
	done

# start
	for i in $(start) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/start $$i ;\
	done

# stop
	for i in $(stop) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR)/stop $$i ;\
	done

	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(INSDIR) $$i ;\
	done

#symbolic link all Help files
	-$(LINK) $(INSDIR)/Help $(INSDIR)/add/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/disable/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/enable/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/list/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/modify/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/remove/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/start/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/stop/Help 

size: all

strip: all

$(INSDIR):
	if [ ! -d $(INSDIR) ] ;\
	then \
		mkdir $(INSDIR) ;\
	fi
$(TASKS):
	if [ ! -d $(INSDIR)/$(@) ] ;\
	then \
		mkdir $(INSDIR)/$(@) ;\
	fi
