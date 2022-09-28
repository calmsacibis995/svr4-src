#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vi:vi.mk	1.35"

#	Makefile for vi

ROOT =

DIR = $(ROOT)/usr/bin

DIRS = $(ROOT)/etc/init.d $(ROOT)/etc/rc2.d

INC = $(ROOT)/usr/include

LDFLAGS =

CFLAGS = -O -I$(INC)

INS = install

STRIP = strip

SIZE = size

MAKEFILE = vi.mk

VIMAKE = $(MAKE) -f $(MKFILE) ROOT=$(ROOT)  DIR=$(DIR)

MKDIR = port
MKFILE = makefile.usg         

ALL : all

all : 
	@echo "\n\t>Making commands."
	cd misc; $(MAKE) ROOT=$(ROOT) all; cd ..
	cd $(MKDIR) ; $(VIMAKE) all ; cd ..
	@echo "Finished compiling..."

install: all $(DIRS)
	cd misc; $(MAKE) ROOT=$(ROOT) DIR=$(DIR) install 
	@echo "\n\t> Installing ex object."
	cd $(MKDIR) ; $(VIMAKE) install
	@echo "\n\t> Creating PRESERVE scripts."
	$(INS) -f $(ROOT)/etc/init.d -m 444 -u root -g sys PRESERVE
	-rm -f $(ROOT)/etc/rc2.d/S02PRESERVE
	-ln -f $(ROOT)/etc/init.d/PRESERVE $(ROOT)/etc/rc2.d/S02PRESERVE

$(DIRS):
	mkdir $@

size: all
	cd misc ; $(MAKE) ROOT=$(ROOT) DIR=$(DIR)  size ; cd ..
	cd $(MKDIR) ; $(VIMAKE) size ; cd ..

strip: all
	cd misc ; $(MAKE) ROOT=$(ROOT) DIR=$(DIR)  strip ; cd ..
	cd $(MKDIR) ; $(VIMAKE) strip ; cd ..

#
# Cleanup procedures
#
clobber: 
	cd misc ; $(MAKE) ROOT=$(ROOT) clobber ; cd ..
	cd port; \
	$(VIMAKE) clobber ;

clean:
	cd misc ; $(MAKE) ROOT=$(ROOT) clean ; cd ..
	cd port; \
	$(VIMAKE) clean ;

#	These targets are useful but optional

partslist:
	cd misc ; $(MAKE) ROOT=$(ROOT) partslist ; cd ..
	cd $(MKDIR) ; $(VIMAKE) partslist ; cd ..

productdir:
	cd misc ; $(MAKE) ROOT=$(ROOT) productdir ; cd ..
	cd $(MKDIR) ; $(VIMAKE) productdir ; cd ..

product:
	cd misc ; $(MAKE) ROOT=$(ROOT) product ; cd ..
	cd $(MKDIR) ; $(VIMAKE) product ; cd ..

srcaudit:
	cd misc ; $(MAKE) ROOT=$(ROOT) srcaudit ; cd ..
	cd $(MKDIR) ; $(VIMAKE) srcaudit ; cd ..
