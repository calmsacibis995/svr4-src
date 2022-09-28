#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/postcomm/postcomm.mk	1.2.2.1"

#
# makefile for the program that sends files to PostScript printers.
#

MAKEFILE=postcomm.mk
ARGS=all

#
# Common source and header files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# postcomm doesn't use floating point arithmetic, so the -f flag isn't needed.
#

CFLAGS=-O -DSYSV -I$(COMMONDIR)

#CFILES=postcomm.c ifdef.c
CFILES=postcomm.c

HFILES=postcomm.h

POSTIO=$(CFILES:.c=.o)

ALLFILES=README $(MAKEFILE) $(HFILES) $(CFILES)


all : postcomm

install : postcomm
	@if [ ! -d "$(BINDIR)" ]; then \
	    mkdir $(BINDIR); \
	    $(CH)chmod 775 $(BINDIR); \
	    $(CH)chgrp $(GROUP) $(BINDIR); \
	    $(CH)chown $(OWNER) $(BINDIR); \
	fi
	install -m 775 -u $(OWNER) -g $(GROUP) -f $(BINDIR) postcomm
#	cp postcomm $(BINDIR)
#	chmod 775 $(BINDIR)/postcomm
#	chgrp $(GROUP) $(BINDIR)/postcomm
#	chown $(OWNER) $(BINDIR)/postcomm

postcomm : $(POSTIO)
	if [ -d "$(DKHOSTDIR)" ]; \
	    then $(CC) $(CFLAGS) -o postcomm $(POSTIO) -Wl,-L$(DKHOSTDIR)/lib -ldk; \
	    else $(CC) $(CFLAGS) -o postcomm $(POSTIO); \
	fi

postcomm.o : $(HFILES)

clean :
	rm -f *.o

clobber : clean
	rm -f postcomm

list :
	pr -n $(ALLFILES) | $(LIST)

