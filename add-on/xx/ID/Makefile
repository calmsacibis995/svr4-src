#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft 
#	Corporation and should be treated as Confidential.

#ident	"@(#)xx:ID/Makefile	1.2.1.1"

STRIPFILES = \
	Master \
	Mfsys \
	Sfsys \
	System
OTHERFILES = \
	Space.c \
	Stubs.c
FILES = $(STRIPFILES) $(OTHERFILES)


all:	$(FILES)

install: all
	for f in $(STRIPFILES); \
	do \
		grep -v "^#" $$f | grep -v "^$$" >../pkg/$$f; \
	done
	cp $(OTHERFILES) ../pkg

clean:
