#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:sccs.mk	6.7.1.6"
ROOT=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
HELPLIB=$(CCSLIB)/help
CFLAGS=-O
LINK_MODE=
ARGS=
STRIP=strip
MAKE=make
LINT = lint

ENVPARAMS = ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" INSDIR="$(INSDIR)" STRIP="$(STRIP)" CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" HELPLIB="$(HELPLIB)" LINK_MODE="$(LINK_MODE)"

all: libs cmds helplib 
	@echo "SCCS is built"

lintit: 
	cd lib; $(MAKE) -f lib.mk $(ENVPARAMS) lintit
	cd cmd; $(MAKE) -f cmd.mk $(ENVPARAMS) lintit
	@echo "SCCS is linted"

libs:
	cd lib; $(MAKE) -f lib.mk $(ENVPARAMS)

cmds:
	@-chmod +x ./chk_fsync
	@-./chk_fsync
	CPPFLAGS=`cat cppargs` ; export CPPFLAGS; cd cmd; $(MAKE) -f cmd.mk $(ENVPARAMS)
	@rm -f cppargs

helplib:
	cd help.d; $(MAKE) -f help.mk $(ENVPARAMS)

install:
	cd lib; $(MAKE) -f lib.mk $(ENVPARAMS) install
	cd cmd; $(MAKE) -f cmd.mk $(ENVPARAMS) $(ARGS) install
	cd help.d; $(MAKE) -f help.mk $(ENVPARAMS) install

clean:
	cd lib; $(MAKE) -f lib.mk $(ENVPARAMS) clean
	cd cmd; $(MAKE) -f cmd.mk $(ENVPARAMS) clean
	cd help.d; $(MAKE) -f help.mk $(ENVPARAMS) clean

clobber:
	cd lib; $(MAKE) -f lib.mk $(ENVPARAMS) clobber
	cd cmd; $(MAKE) -f cmd.mk $(ENVPARAMS) clobber
	cd help.d; $(MAKE) -f help.mk $(ENVPARAMS) clobber 
