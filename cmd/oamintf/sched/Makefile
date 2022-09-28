#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:sched/Makefile	1.1"
OAMBASE=/usr/sadm/sysadm
VMSYS=$(ROOT)/usr/vmsys/OBJECTS
FILEDIR = $(ROOT)$(OAMBASE)/menu/sched
INS = install

TASKS = sched


sched = Form.add Form.chg Menu.chg Menu.day Menu.del Menu.hour Menu.minute Menu.month Menu.sched Menu.week Text.addcfm Text.chgcfm Text.delcfm Text.display Text.noaces Text.nocron Text.nodel Text.notask Help

all: install


clean:

clobber:

install: $(FILEDIR) 
# sched
	-for i in $(sched) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(FILEDIR) $$i ;\
	done
	-ln $(FILEDIR)/Menu.sched $(VMSYS)/sched/Menu.sched

$(FILEDIR):
	-[ -d $(FILEDIR) ] || mkdir -p $(FILEDIR)
	-[ -d $(VMSYS)/sched ] || mkdir -p $(VMSYS)/sched
