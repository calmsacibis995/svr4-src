#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pc586:ID/Makefile	1.3"

STRIPFILES =	Master System Node
OTHERFILES =	Space.c
FILES =	$(STRIPFILES) $(OTHERFILES)

all:	$(FILES)

install:	all
	[ -d ../pkg ] || mkdir ../pkg
	for i in $(STRIPFILES) ; \
	do \
		grep -v "^\*" $$i | grep -v "^$$" > ../pkg/$$i ; \
	done
	cp $(OTHERFILES) ../pkg

clean:
	-for i in $(STRIPFILES) $(OTHERFILES); \
	do \
		rm -f ../pkg/$$i ; \
	done

clobber: clean
