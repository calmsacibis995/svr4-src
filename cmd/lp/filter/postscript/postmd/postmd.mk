#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postmd/postmd.mk	1.2.2.1"

#
# makefile for the matrix display program.
#

MAKEFILE=postmd.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

CFLAGS=-O -DSYSV -I$(COMMONDIR)

CFILES=postmd.c \
       $(COMMONDIR)/request.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c

HFILES=postmd.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

POSTMD=postmd.o\
       $(COMMONDIR)/request.o \
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postmd

install : postmd
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postmd
#	cp postmd $(BINDIR)
#	chmod 775 $(BINDIR)/postmd
#	chgrp $(GROUP) $(BINDIR)/postmd
#	chown $(OWNER) $(BINDIR)/postmd

postmd : $(POSTMD)
	$(CC) $(CFLAGS) -o postmd $(POSTMD) -lm

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/request.o : $(COMMONDIR)/request.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h $(COMMONDIR)/path.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c request.c

postmd.o : $(HFILES)

clean :
	rm -f $(POSTMD)

clobber : clean
	rm -f postmd

list :
	pr -n $(ALLFILES) | $(LIST)

