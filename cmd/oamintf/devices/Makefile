#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:devices/Makefile	1.7.3.1"

DIRS=add copy format erase display remove

O_SHFILES= \
	getdlst showgrp valdrive getdisk valdisk ckdisk devtabcmd listtypes \
	getdevice valdevice
O_DFILES= \
	Help \
	Text.diskerr Text.nodev Menu.disk Menu.drive Menu.device devices.menu \
	Text.rsvrd Text.rsvrd2

all:	$(O_SHFILES)
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f Makefile $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f Makefile $@ ;\
			cd .. ;\
		fi ;\
	done

size strip clean:
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f Makefile $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f Makefile $@ ;\
			cd .. ;\
		fi ;\
	done

clobber: clean
	rm -f $(O_SHFILES)
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f Makefile $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f Makefile $@ ;\
			cd .. ;\
		fi ;\
	done

install: $(O_SHFILES) xinstall 
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -f Makefile $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) -f Makefile $@ ;\
			cd .. ;\
		fi ;\
	done

PROTO=Prototype
OBJECTS=$(O_DFILES) $(O_SHFILES)
include ../Install
