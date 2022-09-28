#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:devices/display/Makefile	1.5.4.1"

O_SHFILES=\
	dispdisk
O_DFILES=\
	Help \
	Menu.display Form.display Text.display Text.disperr Text.disprem

all: $(O_SHFILES)

clean:

clobber: clean
	rm -f $(O_SHFILES)

size strip: all
	:

install: all xinstall

PROTO=Prototype
OBJECTS=$(O_SHFILES) $(O_DFILES)
include ../../Install
