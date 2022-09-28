#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postreverse/postrev.mk	1.2.2.1"

#
# makefile for the page reversal utility program.
#

MAKEFILE=postreverse.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# postreverse doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -I$(COMMONDIR)

CFILES=postreverse.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c \
       $(COMMONDIR)/tempnam.c

HFILES=postreverse.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

POSTREVERSE=postreverse.o\
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o\
       $(COMMONDIR)/tempnam.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postreverse

install : postreverse
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postreverse
#	cp postreverse $(BINDIR)
#	chmod 775 $(BINDIR)/postreverse
#	chgrp $(GROUP) $(BINDIR)/postreverse
#	chown $(OWNER) $(BINDIR)/postreverse

postreverse : $(POSTREVERSE)
	$(CC) $(CFLAGS) -o postreverse $(POSTREVERSE)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/tempnam.o : $(COMMONDIR)/tempnam.c
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c tempnam.c

postreverse.o : $(HFILES)


clean :
	rm -f $(POSTREVERSE)

clobber : clean
	rm -f postreverse

list :
	pr -n $(ALLFILES) | $(LIST)

