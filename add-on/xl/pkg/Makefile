#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:pkg/Makefile	1.1"

FILES=Driver.o \
	Driver.o.fd \
	Files \
	Install \
	Name \
	Node \
	Remove \
	Size \
	Space.c \
	Master \
	System \
	ftape.h \
	ftapecntl\
	Menu.tape \
	Text.tape \
	Text.fop \
	Text.sop \
	admin	

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	rm -rf $(ROOT)/usr/src/pkg/xl
	mkdir $(ROOT)/usr/src/pkg/xl
	ls -t | grep -v Makefile | cpio -pdl $(ROOT)/usr/src/pkg/xl
#	cp $(ROOT)/usr/bin/backup .
	sleep 1
	touch Size
	ls -t | grep -v Makefile | cpio -ocvB > $(ROOT)/usr/src/pkg/$$$$.xl

install clean all: $(FRC)

clobber:
	rm -f $(FILES)

FRC:
