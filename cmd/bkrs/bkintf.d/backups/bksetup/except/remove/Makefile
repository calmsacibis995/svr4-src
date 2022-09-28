#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksetup/except/remove/Makefile	1.3.2.1"
OAMBASE=$(ROOT)/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
REMDIR = $(OAMBASE)/menu/backup_service/setup/exception_list/remove
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = remove

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.remove Text.remove Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(REMDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(REMDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(REMDIR):
	if [ ! -d `dirname $(REMDIR)` ] ;\
	then \
		mkdir -p `dirname $(REMDIR)` ;\
	fi
	if [ ! -d $(REMDIR) ] ;\
	then \
		mkdir -p $(REMDIR) ;\
	fi
