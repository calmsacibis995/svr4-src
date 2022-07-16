#ident	"@(#)Makefile	1.2	92/06/28	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)qt:pkg/Makefile	1.3.2.2"

FILES=Driver.o \
	Files \
	Install \
	Name \
	Node \
	Remove \
	Size \
	Space.c \
	qt.h \
	address \
	Master \
	System \
	tapecntl\
	Menu.tape \
	Text.tape \
	Text.fop \
	Text.sop \
	admin

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	rm -rf $(ROOT)/usr/src/pkg/qt
	mkdir $(ROOT)/usr/src/pkg/qt
	ls -t | grep -v Makefile | cpio -pdl $(ROOT)/usr/src/pkg/qt

install clean all: $(FRC)

clobber:
	rm -f $(FILES)

FRC:
