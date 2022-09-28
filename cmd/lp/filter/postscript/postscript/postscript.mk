#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postscript/postscript.mk	1.2.2.1"

#
# A makefile for installing PostScript library files.
#

MAKEFILE=postscript.mk
ARGS=compile

#
# If $(WANTROUND) is true file $(ROUNDPAGE) is appended to the installed prologue
# files listed in $(ROUNDPROGS).
#

WANTROUND=false
ROUNDPAGE=roundpage.ps
ROUNDPROGS=dpost postprint postdaisy

LIBFILES=*.ps ps.*
ALLFILES=README $(MAKEFILE) $(LIBFILES)


all :

install :
	@if [ ! -d "$(LIBDIR)" ]; then \
	    mkdir $(LIBDIR); \
	    $(CH)chmod 775 $(LIBDIR); \
	    $(CH)chgrp $(GROUP) $(LIBDIR); \
	    $(CH)chown $(OWNER) $(LIBDIR); \
	fi
	for f in $(LIBFILES); \
	do \
		install -m 664 -u $(OWNER) -g $(GROUP) -f $(LIBDIR) $$f; \
	done
#	cp $(LIBFILES) $(LIBDIR)
#	@if [ "$(WANTROUND)" = "true" ]; then \
#	    for i in $(ROUNDPROGS); do \
#		if [ -w $(LIBDIR)/$$i.ps ]; then \
#		    cat $(ROUNDPAGE) >>$(LIBDIR)/$$i.ps; \
#		fi; \
#	    done; \
#	fi
#	cd $(LIBDIR); \
#	chmod 664 $(LIBFILES); \
#	chgrp $(GROUP) $(LIBFILES); \
#	chown $(OWNER) $(LIBFILES)

clean clobber :

list :
	pr -n $(ALLFILES) | $(LIST)

