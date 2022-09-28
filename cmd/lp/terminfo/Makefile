#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:terminfo/Makefile	1.1.1.1"
#
# Makefile for the Terminfo entries delivered with LP
#


TOP	=	..

include ../common.mk


TERMINFO=	$(USRSHARELIB)/terminfo
TITMP	=	$(TERMINFO)/ti


COMPILE	=	tic -v


SRCS	= \
		PS.ti

all:
		
install:	ckdir \
		terminfo.src
	TERMINFO=$(TERMINFO) $(COMPILE) terminfo.src
	cp $(SRCS) $(TITMP)

clean:
	$(RM) terminfo.src

clobber:	clean

strip:

terminfo.src:	$(SRCS)
	@cat $(SRCS) >terminfo.src

ckdir:
	-if [ ! -d $(TITMP) ]; \
	then \
		mkdir -p $(TITMP); \
		$(CH)chown root $(TITMP); \
		$(CH)chgrp root $(TITMP); \
		chmod $(DMODES) $(TITMP); \
	fi
