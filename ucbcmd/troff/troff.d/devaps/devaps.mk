#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbtroff:troff.d/devaps/devaps.mk	1.2.1.1"


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	makefile for aps-5 driver, fonts, etc.
#
# DSL 2

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s $(SHLIBS)
INS = :
INSDIR = $(OL)usr/ucb
FONTHOME = $(OL)usr/ucblib/doctools/font
FONTDIR = $(OL)usr/ucblib/doctools/font/devaps
MAKEDEV = ./makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].[oa][ud][td] [A-Z][0-9A-Z].[oa][ud][td] DESC.out

all:	daps aps_fonts

daps:	daps.o ../draw.o build.o
	$(CC) $(LDFLAGS) $(FFLAG) -o daps daps.o ../draw.o build.o -lm

daps.o:	aps.h ../dev.h daps.h daps.g
	$(CC) $(CFLAGS) -I../ -c daps.c

../draw.o:	../draw.c
	cd ..;  $(MAKE) draw.o

aps_fonts:	$(MAKEDEV)
	$(MAKEDEV) DESC
	for i in $(FFILES); \
	do	if [ ! -r $$i.out ] || [ -n "`find $$i -newer $$i.out -print`" ]; \
		   then	$(MAKEDEV) $$i; \
		fi; \
	done
	-if [ -r LINKFILE ]; then \
	    sh ./LINKFILE; \
	fi

$(MAKEDEV):	$(MAKEDEV).c ../dev.h
	cc $(CFLAGS) -I../ $(LDFLAGS) -o $(MAKEDEV) $(MAKEDEV).c
makedir:
	if [ ! -d $(FONTHOME) ] ; then rm -f $(FONTHOME);  mkdir $(FONTHOME); \
		chmod 755 $(FONTHOME);  fi
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR); \
		chmod 755 $(FONTDIR);  fi

install: daps aps_fonts makedir
	$(INS) -f $(INSDIR) -m 755 -u bin -g bin daps 
	for i in ${OFILES} version; do \
   	    ($(INS) -f $(FONTDIR) -u bin -g bin -m 644 $$i); done

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) daps makedev
