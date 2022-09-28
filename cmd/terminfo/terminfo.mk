#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)terminfo:terminfo.mk	1.10"
#
#	terminfo makefile
#

ROOT=
TERMDIR=$(ROOT)/usr/share/lib/terminfo
TABDIR=$(ROOT)/usr/share/lib/tabset
TERMCAPDIR=$(ROOT)/usr/share/lib
PARTS=	header *.ti trailer
COMPILE=tic -v

all:	ckdir terminfo.src
	TERMINFO=$(TERMDIR) 2>&1 $(COMPILE) terminfo.src > errs
	@touch install
	@echo
	@sh ./ckout
	@echo
	@echo
	@echo
	cp tabset/* $(TABDIR)
	cp *.ti $(TERMDIR)/ti
	cp termcap $(TERMCAPDIR)

install: all

terminfo.src:	$(PARTS)
	@cat $(PARTS) > terminfo.src

clean:
	rm -f terminfo.src install errs nohup.out

clobber: clean

ckdir:
	@echo "terminfo database will be built in $(TERMDIR)."
	@echo "checking for the existence of $(TERMDIR):"
	@echo
	@if [ -d $(TERMDIR) ]; \
	then \
		echo "\t$(TERMDIR) exists"; \
	else  \
		echo "\tbuilding $(TERMDIR)"; \
		mkdir $(TERMDIR); \
		$(CH)chown bin $(TERMDIR); \
		$(CH)chgrp bin $(TERMDIR); \
		chmod 775 $(TERMDIR); \
	fi
	@echo
	@echo
	@echo
	@echo "terminfo database will reference file in $(TABDIR)."
	@echo "checking for the existence of $(TABDIR):"
	@echo
	@if [ -d $(TABDIR) ]; \
	then \
		echo "\t$(TABDIR) exists"; \
	else  \
		echo "\tbuilding $(TABDIR)"; \
		mkdir $(TABDIR); \
		$(CH)chown bin $(TABDIR); \
		$(CH)chgrp bin $(TABDIR); \
		chmod 775 $(TABDIR); \
	fi
	@echo
	@echo
	@echo "terminfo source files will be installed in $(TERMDIR)/ti."
	@echo "checking for the existence of $(TERMDIR)/ti:"
	@echo
	@if [ -d $(TERMDIR)/ti ]; \
	then \
		echo "\t$(TERMDIR)/ti exists"; \
	else  \
		echo "\tbuilding $(TERMDIR)/ti"; \
		mkdir $(TERMDIR)/ti; \
		$(CH)chown root $(TERMDIR)/ti; \
		$(CH)chgrp root $(TERMDIR)/ti; \
		chmod 775 $(TERMDIR)/ti; \
	fi
	@echo
	@echo It will take quite some time to generate $(TERMDIR)/*/*.
	@echo
