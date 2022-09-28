#ident	"@(#)netsel.adm.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)netsel.adm:netsel.adm.mk	1.2.2.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
PKGINST=nsu
PKGSAV=$(ROOT)/var/sadm/pkg/$(PKGINST)/save
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
MENUDIR = $(OAMBASE)/menu/netservices/selection
INSDIR = $(OAMBASE)/add-ons/nsu/netservices/selection
BINDIR = $(OAMBASE)/bin
SHDIR = ./bin
INS = install
LINK = ln
SYMLINK =

TASKS=display modify

O_SHFILES=nslist chgnetconf

display=Text.ns_list

modify=Menu.ns_nid Form.ns_modify Text.ns_modify

all:
	for x in $(O_SHFILES) ;\
	do \
		cp $(SHDIR)/$$x.sh $(SHDIR)/$$x;\
	done

clean:

clobber: clean
	for x in $(O_SHFILES) ;\
	do \
		rm -f $(SHDIR)/$$x ;\
	done

install: all $(TASKS) 
	-[ -d $(MENUDIR) ] || mkdir -p $(MENUDIR) 
	-[ -d $(BINDIR) ] || mkdir -p $(BINDIR) 
	-[ -d $(PKGSAV)/intf_install ] || mkdir -p $(PKGSAV)/intf_install
# display
	for i in $(display) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(INSDIR)/display $$i ;\
	done

# modify
	for i in $(modify) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(INSDIR)/modify $$i ;\
	done

	$(INS) -m 644 -g bin -u bin -f $(INSDIR) Help
	$(INS) -m 644 -g bin -u bin -f $(MENUDIR) selection.menu
	$(INS) -m 644 -g bin -u bin -f $(PKGSAV)/intf_install selection.mi

# link Help files
	-$(LINK) $(INSDIR)/Help $(INSDIR)/display/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/modify/Help 
	-$(SYMLINK) $(INSDIR)/Help $(MENUDIR)/Help 

#sh scripts
	for i in $(O_SHFILES) ;\
	do \
		$(INS) -m 755 -g bin -u bin -f $(BINDIR) $(SHDIR)/$$i ;\
	done

$(O_SHFILES):
	cp $(SHDIR)/$(@).sh $(SHDIR)/$(@)
	echo in sh

$(TASKS):
	-[ -d $(INSDIR)/$(@) ] || mkdir -p $(INSDIR)/$(@)

size:	all

strip:	all

