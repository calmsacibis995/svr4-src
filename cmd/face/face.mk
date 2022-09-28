#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:face.mk	1.3"

#
# The BIG makefile for ViewMaster - should be run as root
#

HOME = home
OWNER = root
VMSYS = $(ROOT)/$(HOME)/vmsys
VMBIN = $(ROOT)/$(HOME)/vmsys/bin
VMLIB = $(ROOT)/$(HOME)/vmsys/lib
LIBPW = gen
CFLAGS = -O
LDFLAGS = -s

all install clean clobber:
	@set -e;\
	cd src;\
	$(MAKE) -f src.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" VMSYS=$(VMSYS) VMBIN=$(VMBIN) VMLIB=$(VMLIB) LIBPW=$(LIBPW) HOME=$(HOME) OWNER=$(OWNER) $@;\
	echo FINISHED
