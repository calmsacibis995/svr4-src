#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/backups/bksetup/except/display/Makefile	1.3.2.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
DIR = $(ROOT)/bin
INC = $(ROOT)/usr/include
DISPDIR = $(OAMBASE)/menu/backup_service/setup/exception_list/display
INSTALL = install
STRIP = strip
SIZE = size
PRODUCT = display

$(PRODUCT): all

.sh:
	cp $< $*

O_DFILES=Form.display Text.display Text.errs Help

all: $(O_SHFILES)

clean:

clobber:

install: all $(DISPDIR)
	for i in $(O_DFILES) ;\
	do \
		$(INSTALL) -m 644 -g bin -u bin -f $(DISPDIR) $$i ;\
	done

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

$(DISPDIR):
	if [ ! -d `dirname $(DISPDIR)` ] ;\
	then \
		mkdir -p `dirname $(DISPDIR)` ;\
	fi
	if [ ! -d $(DISPDIR) ] ;\
	then \
		mkdir -p $(DISPDIR) ;\
	fi
