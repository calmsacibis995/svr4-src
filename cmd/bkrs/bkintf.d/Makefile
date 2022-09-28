#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkintf.d/Makefile	1.1.2.1"

INC=$(ROOT)/usr/include
LOCAL=hdrs
CC=$(PFX)cc
CFLAGS=
MAKE=make

PRODUCTS=backups

all: $(PRODUCTS)

$(PRODUCTS):
	cd $(@); $(MAKE) -$(MAKEFLAGS) all; cd ..

install: all
	for f in $(PRODUCTS) ; \
	do \
		cd $$f ; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done

strip lintit clobber clean touch:
	for f in $(PRODUCTS) ; \
	do \
		cd $$f ; \
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd ..; \
	done
