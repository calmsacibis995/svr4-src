#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/download/download.mk	1.3.2.1"
#
# makefile for the font download.
#

MAKEFILE=download.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# download doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -I$(COMMONDIR)

CFILES=download.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c \
       $(COMMONDIR)/tempnam.c

HFILES=download.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

DOWNLOADER=download.o\
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o\
       $(COMMONDIR)/tempnam.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : download

install : download
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH) chmod 775 $(BINDIR); \
	    $(CH) chgrp $(GROUP) $(BINDIR); \
	    $(CH) chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) download
#	cp download $(BINDIR)
#	$(CH) chmod 775 $(BINDIR)/download
#	$(CH) chgrp $(GROUP) $(BINDIR)/download
#	$(CH) chown $(OWNER) $(BINDIR)/download

download : $(DOWNLOADER)
	$(CC) $(CFLAGS) -o download $(DOWNLOADER)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/tempnam.o : $(COMMONDIR)/tempnam.c
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c tempnam.c

download.o : $(HFILES)


clean :
	rm -f $(DOWNLOADER)

clobber : clean
	rm -f download

list :
	pr -n $(ALLFILES) | $(LIST)

