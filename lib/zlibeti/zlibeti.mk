#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#ident	"@(#)libeti:libeti.mk	1.7"
#

ROOT=
OWN=bin
GRP=bin
AR=ar
CC=cc
CFLAGS=-O
LINT=lint
LINTFLAGS=-u
MAKE=make
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
CCSLIB=$(ROOT)/usr/ccs/lib

ENVPARMS=ROOT="$(ROOT)" \
	 OWN="$(OWN)" \
	 GRP="$(GRP)" \
	 AR="$(AR)" \
	 CC="$(CC)" \
	 CFLAGS="$(CFLAGS)" \
	 LINT="$(LINT)" \
	 LINTFLAGS="$(LINTFLAGS)" \
	 INC="$(INC)" \
	 USRLIB="$(USRLIB)" \
	 MAKE="$(MAKE)" \
	 CCSLIB="$(CCSLIB)"

all:
		@echo "\nMaking all for menu\n"
		cd menu; $(MAKE) all $(ENVPARMS)
		@echo "\nMaking all for form\n"
		cd form; $(MAKE) all $(ENVPARMS)
		@echo "\nMaking all for panel\n"
		cd panel; $(MAKE) all $(ENVPARMS)

install:
		@echo "\nMaking install for menu\n"
		cd menu; $(MAKE) install $(ENVPARMS)
		@echo "\nMaking install for form\n"
		cd form; $(MAKE) install $(ENVPARMS)
		@echo "\nMaking install for panel\n"
		cd panel; $(MAKE) install $(ENVPARMS)
		install -f $(INC) -m 444 -u $(OWN) -g $(GRP) eti.h

lintit:		
		@echo "\nMaking lintit for menu\n"
		cd menu; $(MAKE) lintit $(ENVPARMS)
		@echo "\nMaking lintit for form\n"
		cd form; $(MAKE) lintit $(ENVPARMS)
		@echo "\nMaking lintit for panel\n"
		cd panel; $(MAKE) lintit $(ENVPARMS)

clean:		
		@echo "\nMaking clean for menu\n"
		cd menu; $(MAKE) clean $(ENVPARMS)
		@echo "\nMaking clean for form\n"
		cd form; $(MAKE) clean $(ENVPARMS)
		@echo "\nMaking clean for panel\n"
		cd panel; $(MAKE) clean $(ENVPARMS)

clobber:	clean
		@echo "\nMaking clobber for menu\n"
		cd menu; $(MAKE) clobber $(ENVPARMS)
		@echo "\nMaking clobber for form\n"
		cd form; $(MAKE) clobber $(ENVPARMS)
		@echo "\nMaking clobber for panel\n"
		cd panel; $(MAKE) clobber $(ENVPARMS)
