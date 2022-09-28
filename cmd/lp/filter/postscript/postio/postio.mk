#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postio/postio.mk	1.6.3.1"
#
# makefile for the RS-232 serial interface program for PostScript printers.
#

MAKEFILE=postio.mk
ARGS=all

#
# Common source and header files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# postio doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -DSYSV -I$(COMMONDIR)
SYSTEM=SYSV

CFILES=postio.c ifdef.c slowsend.c

HFILES=postio.h\
       ifdef.h\
       $(COMMONDIR)/gen.h

POSTIO=postio.o ifdef.o slowsend.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postio

install : postio
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postio
#	cp postio $(BINDIR)
#	$(CH)chmod 775 $(BINDIR)/postio
#	$(CH)chgrp $(GROUP) $(BINDIR)/postio
#	$(CH)chown $(OWNER) $(BINDIR)/postio

postio : $(POSTIO)
	@if [ "$(SYSTEM)" = "SYSV" -a -d "$(DKHOSTDIR)" ]; then \
	    EXTRA="-Wl,-L$(DKHOSTDIR)/lib -ldk"; \
	fi; \
	if [ "$(SYSTEM)" = "V9" ]; then \
	    EXTRA="-lipc"; \
	fi; \
	echo "	$(CC) $(CFLAGS) -o postio $(POSTIO) $$EXTRA"; \
	$(CC) $(CFLAGS) -o postio $(POSTIO) $$EXTRA

postio.o : $(HFILES)
slowsend.o : postio.h $(COMMONDIR)/gen.h
ifdef.o : ifdef.h $(COMMONDIR)/gen.h

clean :
	rm -f $(POSTIO)

clobber : clean
	rm -f postio

list :
	pr -n $(ALLFILES) | $(LIST)

