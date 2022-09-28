#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/template/template.mk	1.1.2.1"
#
# template makefile - change all occurrences of template (TEMPLATE) to the name
# (NAME) of your program.
#

MAKEFILE=template.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

CFLAGS=-O -DSYSV -I$(COMMONDIR)

CFILES=template.c \
       $(COMMONDIR)/request.c \
       $(COMMONDIR)/glob.c \
       $(COMMONDIR)/misc.c

HFILES=template.h \
       $(COMMONDIR)/comments.h \
       $(COMMONDIR)/ext.h \
       $(COMMONDIR)/gen.h \
       $(COMMONDIR)/path.h

TEMPLATE=template.o\
       $(COMMONDIR)/request.o\
       $(COMMONDIR)/glob.o\
       $(COMMONDIR)/misc.o

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : template

install : template
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    chmod 775 $(BINDIR); \
	    chgrp $(GROUP) $(BINDIR); \
	    chown $(OWNER) $(BINDIR); \
	fi
	cp template $(BINDIR)
	chmod 775 $(BINDIR)/template
	chgrp $(GROUP) $(BINDIR)/template
	chown $(OWNER) $(BINDIR)/template

template : $(TEMPLATE)
	$(CC) $(CFLAGS) -o template $(TEMPLATE)

$(COMMONDIR)/glob.o : $(COMMONDIR)/glob.c $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c glob.c

$(COMMONDIR)/misc.o : $(COMMONDIR)/misc.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c misc.c

$(COMMONDIR)/request.o : $(COMMONDIR)/request.c $(COMMONDIR)/ext.h $(COMMONDIR)/gen.h $(COMMONDIR)/path.h
	cd $(COMMONDIR); $(CC) $(CFLAGS) -c request.c

template.o : $(HFILES)

clean :
	rm -f $(TEMPLATE)

clobber : clean
	rm -f template

list :
	pr -n $(ALLFILES) | $(LIST)

