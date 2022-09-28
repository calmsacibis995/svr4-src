#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:portmgmt.mk	1.6.3.1"

DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
OAMBASE=/usr/sadm/sysadm
BINDIR = $(ROOT)$(OAMBASE)/bin
PORTSDIR = $(ROOT)$(OAMBASE)/menu/ports
SHDIR = ./bin
STRIP = strip
SIZE = size
INSTALL = install

DIRS = port_monitors port_services tty_settings port_quick

O_SHFILES = \
	ckbaud \
	ckwcount \
	ckfile \
	findpmtype \
	lsopts \
	modifypm \
	pmadmopts \
	pmgetpid \
	psmod \
	pstest \
	sacopts \
	settinglist \
	tmopts \
	uniq_pmtag \
	uniq_svctag \
	uniq_label

all: $(O_SHFILES) 
	 @for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f *.mk $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f *.mk $@ ;\
			cd .. ;\
		fi ;\
	done

install: all $(PORTSDIR) 
	$(INSTALL) -m 644 -g bin -u bin -f $(PORTSDIR) ports.menu
	$(INSTALL) -m 644 -g bin -u bin -f $(PORTSDIR) Help
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f *.mk $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f *.mk $@ ;\
			cd .. ;\
		fi ;\
	done

#sh scripts
	for i in $(O_SHFILES) ;\
	do \
		$(INSTALL) -m 755 -g bin -u bin -f $(BINDIR) $(SHDIR)/$$i ;\
	done

$(O_SHFILES):
	cp $(SHDIR)/$(@).sh $(SHDIR)/$(@)

$(PORTSDIR):
	if [ ! -d `dirname $(PORTSDIR)` ] ;\
	then \
		mkdir `dirname $(PORTSDIR)` ;\
	fi
	if [ ! -d $(PORTSDIR) ] ;\
	then \
		mkdir $(PORTSDIR) ;\
	fi

clean clobber: 
	for x in $(O_SHFILES) ;\
	do \
		rm -f $(SHDIR)/$$x ;\
	done

	 @for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f *.mk $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f *.mk $@ ;\
			cd .. ;\
		fi ;\
	done

size: all
	$(SIZE)
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f *.mk $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f *.mk $@ ;\
			cd .. ;\
		fi ;\
	done

strip: all
	$(STRIP)
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f *.mk $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f *.mk $@ ;\
			cd .. ;\
		fi ;\
	done

