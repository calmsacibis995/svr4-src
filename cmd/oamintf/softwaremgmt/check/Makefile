#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:softwaremgmt/check/Makefile	1.3.1.2"

O_DFILES=\
	task.inst task.orig task.spool \
	Form.inst Form.orig Form.spool check.menu
HELPFILES=\
	Help
HELPDEST=\
	check/Help

all: $(O_SHFILES)

clean:
	:

clobber: clean
	:

size strip: all
	:

install: all xinstall

PROTO=../Prototype
OBJECTS=$(HELPFILES) $(O_SHFILES) $(O_DFILES)
OBJDEST=$(HELPDEST)
include ../../Install
