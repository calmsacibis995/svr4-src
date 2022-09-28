#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-streams:streams.mk	1.3.3.2"

ROOT =
INCSYS = $(ROOT)/usr/include
INC =  $(ROOT)/usr/include
INS = install
SYMLINK = :

all install:
	cd log ; $(MAKE) -f str.mk "ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" "INCSYS=$(INCSYS)" "INC=$(INC)" "INS=$(INS)" "SYMLINK=$(SYMLINK)" $@
	cd strcmd ; $(MAKE) -f strcmd.mk "ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" "INCSYS=$(INCSYS)" "INC=$(INC)" "INS=$(INS)" "SYMLINK=$(SYMLINK)" $@
	cd kmacct ; $(MAKE) -f kmacct.mk "ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" "INCSYS=$(INCSYS)" "INC=$(INC)" "INS=$(INS)" "SYMLINK=$(SYMLINK)" $@

clean clobber:
	cd log ; $(MAKE) -f str.mk $@
	cd strcmd ; $(MAKE) -f strcmd.mk $@
	cd kmacct ; $(MAKE) -f kmacct.mk $@
