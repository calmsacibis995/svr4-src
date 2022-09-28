#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)optim:i386/makefile	1.1.2.6"
#
#	iAPX386 OPTIMIZER MAKEFILE
#
#
ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-c -O
LDLIBS=
LINT=lint
LINTFLAGS=$(DEFLIST) $(INCLIST)

INC=$(ROOT)/usr/include
CCSLIB=$(ROOT)/usr/ccs/lib
SGSBASE=../..
I386INC=$(SGSBASE)/inc/i386
COMINC=$(SGSBASE)/inc/common
COMDIR=../common

INS=$(SGSBASE)/sgs.install
INSDIR=$(CCSLIB)

STRIP=strip

SOURCES=$(COMDIR)/inter.c $(COMDIR)/optim.c \
	debug.c inline.c local.c optutil.c \
	peep.c regal.c w1opt.c w2opt.c w3opt.c w4opt.c w6opt.c
OBJECTS=debug.o inter.o optim.o inline.o local.o optutil.o \
	peep.o regal.o w1opt.o w2opt.o w3opt.o w4opt.o w6opt.o
PRODUCTS=optim

DEFLIST=
INCLIST= -I. -I$(COMDIR) -I$(I386INC) -I$(COMINC)  -I$(INC)
CC_CMD=$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)
LD_CMD=$(CC) $(LDFLAGS)
LINT_CMD=$(LINT) $(LINTFLAGS)

all:		$(PRODUCTS)

optim:		$(OBJECTS)
		$(LD_CMD) $(OBJECTS) $(LDLIBS) -o optim

inter.o:	$(COMDIR)/optim.h defs $(COMDIR)/inter.c
		$(CC_CMD) $(COMDIR)/inter.c

optim.o:	$(COMDIR)/optim.h defs $(COMDIR)/optim.c
		$(CC_CMD) $(COMDIR)/optim.c

inline.o:	$(COMDIR)/optim.h defs optutil.h $(I386INC)/paths.h \
		inline.c
		$(CC_CMD) inline.c

debug.o:	defs debug.h regal.h debug.c
		$(CC_CMD) debug.c

local.o:	$(COMDIR)/optim.h defs optutil.h $(I386INC)/sgs.h \
		$(I386INC)/paths.h regal.h $(COMINC)/storclass.h local.c
		$(CC_CMD) local.c

optutil.o:	$(COMDIR)/optim.h defs optutil.c
		$(CC_CMD) optutil.c

peep.o:		$(COMDIR)/optim.h defs peep.c
		$(CC_CMD) peep.c

regal.o:	$(COMDIR)/optim.h defs optutil.h regal.h \
		$(COMINC)/storclass.h regal.c
		$(CC_CMD) regal.c

w1opt.o:	$(COMDIR)/optim.h defs optutil.h w1opt.c
		$(CC_CMD) w1opt.c

w2opt.o:	$(COMDIR)/optim.h defs optutil.h w2opt.c
		$(CC_CMD) w2opt.c

w3opt.o:	$(COMDIR)/optim.h defs optutil.h w3opt.c
		$(CC_CMD) w3opt.c

w4opt.o:	$(COMDIR)/optim.h defs optutil.h w4opt.c
		$(CC_CMD) w4opt.c

w6opt.o:	$(COMDIR)/optim.h defs optutil.h w6opt.c
		$(CC_CMD) w6opt.c

install:	$(INSDIR)/$(SGS)optim

$(INSDIR)/$(SGS)optim:	all
		cp optim optim.bak
		-$(STRIP) optim
		/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)optim optim
		mv optim.bak optim

save:		$(INSDIR)/$(SGS)optim
		-rm -f $(INSDIR)/$(SGS)optim.back
		cp $(INSDIR)/$(SGS)optim $(INSDIR)/$(SGS)optim.back

uninstall:	$(INSDIR)/$(SGS)optim.back
		-rm -f $(INSDIR)/$(SGS)optim
		cp $(INSDIR)/$(SGS)optim.back $(INSDIR)/optim

lintit:		$(SOURCES)
		$(LINT_CMD) $(SOURCES)

clean:
		-rm -f $(OBJECTS)

shrink:		clobber

clobber:	clean
		-rm -f $(PRODUCTS)
