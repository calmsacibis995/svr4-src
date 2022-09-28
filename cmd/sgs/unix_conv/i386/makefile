#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)unix_conv:i386/makefile	1.2"

#***************************************************************************
#                                                                          *
#		UNIX 5.0 Transition Aid (convert) makefile	 	   *
#									   *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#	inc/ar.h		archive file format header		   *
#	inc/a.out.h		machine language file format header        *
#	inc/sgs.h		sgs primitives				   *
#	com/old.a.out.h		pre 5.0 headers for a.out.h		   *
#	com/old.ar.h		pre 5.0 headers for ar.h		   *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	com/convert.c		transition aid source code		   *
#									   *
#***************************************************************************

LPT = dpr -mx

OUTFIL = output.tmp

ROOT	=
OWN	= bin
GRP	= bin
SGS	= i386
MACH	= i386
LDLIBS=
CCSBIN	= $(ROOT)/usr/ccs/bin

SGSBASE	= ../..
COMINC = $(SGSBASE)/inc/common
CDIR = ../common
MACHINC = $(SGSBASE)/inc/$(MACH)
LINK_MODE = 


OFILES = convert.o

CFILES = $(CDIR)/convert.c

HFILES = $(COMINC)/ar.h $(COMINC)/aouthdr.h $(COMINC)/filehdr.h \
	 $(COMINC)/scnhdr.h $(COMINC)/linenum.h $(COMINC)/reloc.h \
	 $(COMINC)/syms.h $(COMINC)/storclass.h \
	 $(CDIR)/old.a.out.h $(CDIR)/old.ar.h $(CDIR)/5.0.ar.h \
	 $(MACHINC)/paths.h

LIBLD	= $(SGSBASE)/libld/i386/libld.a
STRIP	= strip

FLEX	= -DFLEXNAMES
DEFLIST	= -Di386 $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(CDIR) -I$(COMINC) -I$(MACHINC)

CFLAGS = -O

ARFORMAT=PORTAR


all:	convert


convert:$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) $(LINK_MODE) $(CFILES) $(LIBLD) $(LDLIBS) -o convert


install:	convert
	cp convert convert.bak
	$(STRIP) convert
	sh $(SGSBASE)/sgs.install 755 $(OWN) $(GRP) $(CCSBIN)/convert convert
	mv convert.bak convert

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f $(OFILES)

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

shrink:  clobber

clobber: clean
	-rm -f convert
