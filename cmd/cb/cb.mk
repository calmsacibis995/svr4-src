#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cb:m32/makefile	1.3.3.1"
#
#	CB MAKEFILE
#
#

ROOT=
SGS=
OWN=bin
GRP=bin
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include

MAKE=make
STRIP=strip
LIBELF=
LDLIBS=
LINK_MODE=

CC= cc
CFLAGS=-O 

LINT= lint
LINT_CMD=$(LINT) $(USRINC)

USRBIN=$(ROOT)/usr/bin
INSDIR=$(CCSBIN)
VERSION =

CMDBASE=..
INS=$(CMDBASE)/install/install.sh
SGSBASE=../sgs
MACH=
MACHINC=$(SGSBASE)/inc/$(MACH)
HFILE= $(MACHINC)/sgs.h
INCLIST= -I$(MACHINC) -I$(INC)

CFILE = cb.c
PRODUCTS=cb

all:	$(PRODUCTS)

$(PRODUCTS):	specific
	$(CC) cb.o $(LINK_MODE) -o $(PRODUCTS)

specific:
	if u3b2; \
	then $(MAKE) -f cb.mk MACH=m32 CC=$(CC) CFLAGS=$(CFLAGS) INC=$(INC) cb.o ;\
	elif i386 ;\
	then $(MAKE) -f cb.mk MACH=i386 CC=$(CC) CFLAGS=$(CFLAGS) INC=$(INC) cb.o ;\
	fi

cb.o:	$(CFILE) $(HFILE)
	$(CC) $(CFLAGS) $(INCLIST) -c $(CFILE) 


install: all
	 cp cb cb.bak
	 $(STRIP) cb
	 /bin/sh $(INS) -f $(INSDIR) cb
	 mv cb.bak cb

clean:
	 -rm -f *.o

clobber: clean
	-rm -f cb

lintit:	
	if u3b2; \
	then $(MAKE) -f cb.mk MACH=m32 LINT=$(LINT) INC=$(INC) speclint ;\
	elif i386  ;\
	then $(MAKE) -f cb.mk MACH=i386 LINT=$(LINT) INC=$(INC) speclint ;\
	fi

speclint:	$(CFILE) $(HFILE)
	$(LINT_CMD) $(INCLIST) $(CFILE)
