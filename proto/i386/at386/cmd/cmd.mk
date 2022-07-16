#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto:i386/at386/cmd/cmd.mk	1.3.1.1"

#       Makefile for cmd directory

INSTALLS=INSTALL3
PACKAGE_SCRIPTS=make_flops ask_drive boot_make mini_kernel
ALL=readfloppy $(PACKAGE_SCRIPTS)

all: $(INSTALLS) $(ALL) $(ROOT)/xenv/mult.pkg

$(ROOT)/xenv/mult.pkg: mult.pkg
	cp mult.pkg $(ROOT)/xenv/mult.pkg
	chmod 755 $(ROOT)/xenv/mult.pkg

install: $(INSTALLS) $(ALL)
	cp $? ..

clean:
	rm -f *.o

clobber: clean
	rm -f $(ALL)
