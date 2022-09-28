#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/lib.mk	6.8"
#
#

CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

LIBS=	llib-lcassi.a \
	llib-lcom.a \
	llib-lmpw.a

LLIBS=	llib-lcassi.ln \
	llib-lcom.ln \
	llib-lmpw.ln

PRODUCTS = $(LLIBS)

INSTR=

LINT = lint

LINTFLAGS = 

MAKE = make

INC=
INCSYS=

ENVPARAMS = ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" LINK_MODE="$(LINK_MDOE)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INC="$INC" INCSYS="$INCSYS" INS="$(INS)" INSDIR="$(INSDIR)" STRIP="$(STRIP)"

all:	$(LIBS)
	@echo "SCCS libraries are up to date."

lintit:	$(LLIBS)
	@echo "SCCS lint libraries are up to date."

llib-lcassi.a:
	cd cassi; ${MAKE} -f cassi.mk $(ENVPARAMS)

llib-lcom.a:
	cd comobj; $(MAKE) -f comobj.mk $(ENVPARAMS)

llib-lmpw.a:
	cd mpwlib; $(MAKE) -f mpwlib.mk $(ENVPARAMS)

llib-lcassi.ln::
	cd cassi; $(MAKE) -f cassi.mk $(ENVPARAMS) lintit

llib-lcom.ln::
	cd comobj; $(MAKE) -f comobj.mk $(ENVPARAMS) lintit

llib-lmpw.ln::
	cd mpwlib; $(MAKE) -f mpwlib.mk $(ENVPARAMS) lintit

install:
	cd cassi; $(MAKE) -f cassi.mk $(ENVPARAMS) install
	cd comobj; $(MAKE) -f comobj.mk $(ENVPARAMS) install
	cd mpwlib; $(MAKE) -f mpwlib.mk $(ENVPARAMS) install

clean:
	cd cassi; $(MAKE) -f cassi.mk $(ENVPARAMS) clean
	cd comobj; $(MAKE) -f comobj.mk $(ENVPARAMS) clean
	cd mpwlib; $(MAKE) -f mpwlib.mk $(ENVPARAMS) clean

clobber:
	cd cassi; $(MAKE) -f cassi.mk $(ENVPARAMS) clobber
	cd comobj; $(MAKE) -f comobj.mk $(ENVPARAMS) clobber
	cd mpwlib; $(MAKE) -f mpwlib.mk $(ENVPARAMS) clobber

.PRECIOUS:	$(PRODUCTS)
