#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:devices/format/Makefile	1.6.2.2"

O_SHFILES=
O_DFILES=\
	Help \
	Menu.format Form.format Text.format Text.fmtok Text.fmterr \
	Menu.density

all: $(O_SHFILES)

clean:

clobber: clean
	:

size strip: all
	:	

install: all xinstall

PROTO=Prototype
OBJECTS=$(O_SHFILES) $(O_DFILES)
include ../../Install
