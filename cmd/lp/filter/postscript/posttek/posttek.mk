#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/posttek/posttek.mk	1.2.2.1"

#
# makefile for the tektronix 4014 file to PostScript translator.
#

MAKEFILE=posttek.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# posttek doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -DSYSV -I$(COMMONDIR)

CFILES=posttek.c \
       $(COMMONDIR)/request.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c

HFILES=posttek.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

POSTTEK=posttek.o\
       $(COMMONDIR)/request.o \
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : posttek

install : posttek
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) posttek
#	cp posttek $(BINDIR)
#	chmod 775 $(BINDIR)/posttek
#	chgrp $(GROUP) $(BINDIR)/posttek
#	chown $(OWNER) $(BINDIR)/posttek

posttek : $(POSTTEK)
	$(CC) $(CFLAGS) -o posttek $(POSTTEK)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/request.o : $(COMMONDIR)/request.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h $(COMMONDIR)/path.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c request.c

posttek.o : $(HFILES)

clean :
	rm -f $(POSTTEK)

clobber : clean
	rm -f posttek

list :
	pr -n $(ALLFILES) | $(LIST)

