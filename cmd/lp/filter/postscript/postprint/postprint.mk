#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postprint/postprint.mk	1.2.2.1"
#
# makefile for the ASCII file to PostScript translator.
#

MAKEFILE=postprint.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# postprint doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -DSYSV -I$(COMMONDIR)

CFILES=postprint.c \
       $(COMMONDIR)/request.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c

HFILES=postprint.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

POSTPRINT=postprint.o\
       $(COMMONDIR)/request.o\
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postprint

install : postprint
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postprint
#	cp postprint $(BINDIR)
#	chmod 775 $(BINDIR)/postprint
#	chgrp $(GROUP) $(BINDIR)/postprint
#	chown $(OWNER) $(BINDIR)/postprint

postprint : $(POSTPRINT)
	$(CC) $(CFLAGS) -o postprint $(POSTPRINT)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/request.o : $(COMMONDIR)/request.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h $(COMMONDIR)/path.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c request.c

postprint.o : $(HFILES)

clean :
	rm -f $(POSTPRINT)

clobber : clean
	rm -f postprint

list :
	pr -n $(ALLFILES) | $(LIST)

