#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto:i386/PACKAGES/peruser/peruser.mk	1.3"

all: preremove

package: install
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	rm -rf $(ROOT)/usr/src/pkg/peruser
	mkdir $(ROOT)/usr/src/pkg/peruser
	ls preremove pkginfo postinstall prototype | cpio -pdum $(ROOT)/usr/src/pkg/peruser

install: all

preremove: update.c
	$(CC) -O -s -o preremove -DMAX_LIM=100 -DDFLT_LEVEL=16 update.c -lld

../../initial: update.c
	$(CC) -O -s -o ../../initial -DMAX_LIM=16 -DDFLT_LEVEL=16 -DBOOTFLOP update.c -dy

clobber clean:
	rm -rf update.o preremove update
