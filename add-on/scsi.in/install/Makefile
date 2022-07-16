#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:install/Makefile	1.3"

STRIPFILES = \
	Name \
	Size \
	address

OTHERFILES = \
	Install \
	Remove 

FILES =	$(STRIPFILES) $(OTHERFILES)

all:	$(FILES)

install: all
	[ -d ../pkg ] || mkdir ../pkg
	-for f in $(STRIPFILES); \
	do \
		grep -v "^#" $$f | grep -v "^$$" >../pkg/$$f; \
	done
	cp $(OTHERFILES) ../pkg

clean: $(FRC)

clobber: clean
	rm -f $(STRIPFILES)

FRC:
