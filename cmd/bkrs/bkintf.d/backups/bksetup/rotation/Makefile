#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksetup/rotation/Makefile	1.4.2.1"

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
SETDIR = $(OAMBASE)/menu/backup_service/setup/rotation
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = rotation

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.rotate Form.values Text.rotok Text.rotnok Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(SETDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(SETDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(SETDIR):
	if [ ! -d `dirname $(SETDIR)` ] ;\
	then \
		mkdir -p `dirname $(SETDIR)` ;\
	fi
	if [ ! -d $(SETDIR) ] ;\
	then \
		mkdir -p $(SETDIR) ;\
	fi
