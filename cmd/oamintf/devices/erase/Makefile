#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:devices/erase/Makefile	1.4.2.2"

DIR = $(ROOT)/usr/sadm/sysadm/bin
INC = $(ROOT)/usr/include
LDFLAGS = -s
CFLAGS = -O -I$(INC)
FILE = spclsize
O_SHFILES=\
	floperase
O_DFILES=\
	Help \
	Menu.erase Form.erase \
	Text.eraseok Text.erase Text.eraserr

all: $(O_SHFILES) $(FILE)

$(FILE): $(FILE).o
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS) 

$(FILE).o:	./cmderr.h ./stddef.h \
		$(INC)/fcntl.h $(INC)/stdio.h \
		$(INC)/signal.h $(INC)/values.h 

clean:
	rm -fr *.o

clobber: clean
	rm -f $(O_SHFILES)
	rm -f $(FILE)

strip size: all
	:

install: all xinstall
	cp $(FILE) $(DIR)
	$(CH)chmod +x,u+w,go-w $(DIR)/$(FILE)

PROTO=Prototype
OBJECTS=$(O_SHFILES) $(O_DFILES)
include ../../Install
