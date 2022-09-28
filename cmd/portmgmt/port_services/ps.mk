#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:port_services/ps.mk	1.6.2.1"

OAMBASE=/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)$(OAMBASE)/menu/ports/port_services
BINDIR = $(ROOT)$(OAMBASE)/bin
INSTALL = install
LINK = ln

MAINS=pmckmod
PMCKMOD=pmckmod.c
PMCKMODOBJ=pmckmod.o
PMCKMOD=pmckmod

TASKS = add disable enable list modify remove

O_DFILES = ps.menu Menu.c_labels Help

add=Menu.ps_add Form.ps_add Menu.ps_atag Menu.ps_atype Form.ps_addls \
	Form.ps_addtm Form.ps_adduk Text.ps_msg

disable=Menu.ps_disabl

enable=Menu.ps_enable

list=Menu.ps_list Menu.ps_lstag Menu.ps_lstype Text.ps_list Text.ps_lsall

modify=Menu.ps_modify Form.ps_modls Form.ps_modtm 

remove=Menu.ps_remove

all: $(MAINS)

$(PMCKMOD): $(PMCKMODOBJ)
	$(CC) -s -o $(PMCKMOD) $(PMCKMODOBJ) $(SHLIBS)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(MAINS)
	
install: all $(INSDIR) $(TASKS)
	$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) $(PMCKMOD)
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
