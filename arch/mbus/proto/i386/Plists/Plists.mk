#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1989  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:proto/i386/Plists/Plists.mk	1.3"

BINARY	  = $(ROOT)/opt/unix

all:  dirs  Plist Plist.dev 
	@echo "Perms complete"

install: all  
	install -f $(BINARY) -m 644 -u root -g sys Plist
	install -f $(BINARY) -m 644 -u root -g sys Plist.dev

set: install base dev 

base: Plist
	-cd $(ROOT); $(ROOT)/etc/setmods  $(BINARY)/Plist; :

dev: Plist.dev
	-cd $(ROOT); $(ROOT)/etc/setmods -s $(BINARY)/Plist.dev; :

Plist:   Plist.base Plist.$(BUS)
	cp Plist.base Plist
	cat Plist.$(BUS) >> Plist

Plist.dev: Plist.$(BUS).dev Plist.base.dev
	cp Plist.base.dev Plist.dev
	cat Plist.$(BUS).dev >>Plist.dev

Plist.$(BUS) Plist.$(BUS).dev Plist.base.dev:

Plist.base:
	sh get-new-plist

dirs:
	-@[ -d $(BINARY) ]	|| mkdir -p  $(BINARY) || :

clean: 
	rm -f Plist Plist.dev

clobber: clean
	rm -f $(BINARY)/Plist $(BINARY)/Plist.dev Plist.base

FRC:
