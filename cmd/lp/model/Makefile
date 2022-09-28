#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:model/Makefile	1.18.3.1"
#
# Makefile for lp/model
#


TOP	=	..

include ../common.mk


MODELDIR=	$(USRLIBLP)/model

FMODES	=	0444


DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC)


SRCS	= \
		lp.set.c \
		lp.cat.c \
		lp.tell.c \
		drain.output.c

#
# Don't use $(SRCS:.c=.o) because it doesn't work on lp.cat.c
OBJS	= \
		lp.set.o \
		lp.cat.o \
		lp.tell.o \
		drain.output.o

CMDS	= \
		lp.set \
		lp.cat \
		lp.tell \
		drain.output

MODELS	= \
		standard

MISC	= \
		alert.proto


all:		$(CMDS)

install:
	for cmd in $(CMDS); \
	do \
	    $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(LPBINDIR) $$cmd; \
	done
	for model in $(MODELS); \
	do \
	    $(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(MODELDIR) $$model; \
	done
	for misc in $(MISC); \
	do \
	    $(INS) -m $(FMODES) -u $(OWNER) -g $(GROUP) -f $(LPBINDIR) $$misc; \
	done

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMDS)

strip:
	$(STRIP) $(CMDS)

lp.set:		lp.set.o \
			$(LIBLP)
	$(CC) -o lp.set lp.set.o $(LDFLAGS) \
			$(LIBLP) \
			$(NOSHLIBS) \
			-lcurses

lp.cat:		lp.cat.o \
			$(LIBLP)
	$(CC) -o lp.cat lp.cat.o $(LDFLAGS) \
			$(LIBLP) \
			$(NOSHLIBS) \
			-lcurses

lp.tell:	lp.tell.o \
			$(LIBMSG) \
			$(LIBLP)
	$(CC) -o lp.tell lp.tell.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBLP) \
			$(NOSHLIBS) \
			-lcurses

drain.output:	drain.output.o \
			$(LIBLP)
	$(CC) -o drain.output drain.output.o $(LDFLAGS) \
			$(LIBLP) \
			$(NOSHLIBS) \
			-lcurses

lint:
	$(LINT) $(CFLAGS) lp.set.c \
			$(LINTLP)
	$(LINT) $(CFLAGS) lp.cat.c
	$(LINT) $(CFLAGS) lp.tell.c \
			$(LINTLP) \
			$(LINTMSG)
	$(LINT) $(CFLAGS) drain.output.c
