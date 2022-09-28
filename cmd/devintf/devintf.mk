#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devintf:devintf.mk	1.3.4.1"

OAMBASE=$(ROOT)/usr/sadm/sysadm
DESTDIR=$(OAMBASE)/menu/devices
SUBMAKES=devices groups mkdtab
ARCH=AT386
BUS=AT386


foo	: all

.DEFAULT	:	
		for submk in $(SUBMAKES) ; \
		do \
			cd $$submk ; \
			$(MAKE) -f $$submk.mk ARCH=$ARCH BUS=$BUS $@ ; \
			cd .. ; \
		done
