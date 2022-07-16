#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pc586:sys/Makefile	1.3"

DRIVER = pc586
FILES = pc586.h 82586.h lihdr.h
all:	$(FILES)

install: all
	[ -d ../pkg/sys ] || mkdir ../pkg/sys
	for h in $(FILES); \
	do \
	cp $$h ../pkg/sys ; \
	done

clean:
	for h in $(FILES) ; \
	do \
	rm -rf ../pkg/sys/$$h ; \
	done

clobber: clean
