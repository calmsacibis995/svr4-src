#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postdmd/postdmd.mk	1.2.2.1"

#
# makefile for the DMD bitmap translator.
#

MAKEFILE=postdmd.mk
ARGS=compile

#
# Common header perhaps source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# postdmd doesn't use floating point arithemtic, so the -f flag isn't needed.
#

CFLAGS=-O -DSYSV -I$(COMMONDIR)

CFILES=postdmd.c \
       $(COMMONDIR)/request.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c

HFILES=$(COMMONDIR)/comments.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

POSTDMD=postdmd.o\
       $(COMMONDIR)/request.o \
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postdmd

install : postdmd
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postdmd
#	cp postdmd $(BINDIR)
#	chmod 775 $(BINDIR)/postdmd
#	chgrp $(GROUP) $(BINDIR)/postdmd
#	chown $(OWNER) $(BINDIR)/postdmd

postdmd : $(POSTDMD)
	$(CC) $(CFLAGS) -o postdmd $(POSTDMD)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/request.o : $(COMMONDIR)/request.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h $(COMMONDIR)/path.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c request.c

postdmd.o : $(HFILES)

clean :
	rm -f $(POSTDMD)

clobber : clean
	rm -f postdmd

list :
	pr -n $(ALLFILES) | $(LIST)

