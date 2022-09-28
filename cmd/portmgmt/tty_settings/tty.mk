#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:tty_settings/tty.mk	1.6.3.1"

OAMBASE=/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)$(OAMBASE)/menu/ports/tty_settings
BINDIR = $(ROOT)$(OAMBASE)/bin
INS = install
LINK = ln

MAINS=ttylist
TTYSRC=ttylist.c
TTYOBJ=ttylist.o
TTYLIST=ttylist

TASKS = add list remove

O_DFILES = tty.menu ttyvalues Help

add=Form.tty_add Text.tty_add Menu.tty_baud Menu.c_labels

list=Menu.tty_list Text.tty_list

# remove=Menu.tty_remove

all: $(MAINS)

$(TTYLIST): $(TTYOBJ)
	$(CC) -s -o $(TTYLIST) $(TTYOBJ) $(SHLIBS)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(MAINS) Menu.tty_rm
	
install: all $(INSDIR) $(TASKS)
	$(INS) -m 755 -g bin -u bin -f $(BINDIR) $(TTYLIST)
# add
	for i in $(add) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(INSDIR)/add $$i ;\
	done

# list
	for i in $(list) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(INSDIR)/list $$i ;\
	done

# remove
	cp Menu.tty_remove Menu.tty_rm
	$(INS) -m 644 -g bin -u bin -f $(INSDIR)/remove Menu.tty_rm

	for i in $(O_DFILES) ;\
	do \
		$(INS) -m 644 -g bin -u bin -f $(INSDIR) $$i ;\
	done

#symbolic link all Help files
	-$(LINK) $(INSDIR)/Help $(INSDIR)/add/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/list/Help 
	-$(LINK) $(INSDIR)/Help $(INSDIR)/remove/Help 

size:	all

strip:	all

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
