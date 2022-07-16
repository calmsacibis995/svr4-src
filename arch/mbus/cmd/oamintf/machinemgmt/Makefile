#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mbus:cmd/oamintf/machinemgmt/Makefile	1.3"

DIR= $(ROOT)/usr/sadm/sysadm/bin
INC=$(ROOT)/usr/include
LDFLAGS= -s
CFLAGS= -O -I$(INC)
INSTALL = install
O_CFILES= prtconf
O_SHFILES=\
	chgstate
O_DFILES=\
	machine.menu \
	Form.power Text.power \
	Form.reboot Text.reboot \
	Text.whoson

HELPFILES=\
	Help \
	power.Help reboot.Help whoson.Help

HELPDEST=\
	machinemgmt/Help \
	power/Help reboot/Help whoson/Help

all: $(O_SHFILES) $(O_CFILES)

clean:
	rm -rf *.o

clobber: clean
	rm -f $(O_SHFILES) $(O_CFILES)

$(O_CFILES): $(O_CFILES).o
	$(CC) $(CFLAGS) -o $@ $@.o -lmb2 $(LDFLAGS)

size strip: all
	:

install: all xinstall
	$(INSTALL) -u root -g sys -m 555 -f $(DIR) $(O_CFILES)

PROTO=Prototype
OBJECTS=$(HELPFILES) $(O_SHFILES) $(O_DFILES)
OBJDEST=$(HELPDEST)
include ../Install
