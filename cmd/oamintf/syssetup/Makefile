#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:syssetup/Makefile	1.4.2.2"

HELPFILES=Help nodename.Help datetime.Help password.Help

HELPDEST=syssetup/Help nodename/Help datetime/Help password/Help

O_SHFILES=\
	dateset gettz syspwck syspwend
O_CFILES=\
	datechk
O_MSGFILES=\
	nextstep
O_DFILES=\
	syssetup.menu \
	datetime.menu nodename.menu \
	Menu.timezones Text.datenok Text.dateok \
	Text.nodenotok Text.nodeokay \
	Form.syspasswd Form.chgpasswd Menu.syspasswd Text.nologins \
	Text.apswdok Text.apswdnok \
	Form.dateset Text.datedisp Form.nodeset Text.nodedisp

all: $(O_SHFILES) $(O_MSGFILES) $(O_CFILES)

$(O_CFILES):	datechk.c
	$(CC) -s -o datechk datechk.c

$(O_MSGFILES):
	grep -v "^#ident" id.$(@) > $(@)

clean:

clobber:
	rm -f $(O_SHFILES) $(O_MSGFILES) $(O_CFILES)

size strip: all
	:

install: all xinstall

PROTO=Prototype
OBJECTS=$(HELPFILES) $(O_SHFILES) $(O_DFILES) $(O_MSGFILES) $(O_CFILES)
OBJDEST=$(HELPDEST)

include ../Install
