#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/Makefile	1.17.2.1"
#
# Makefile for LP private libraries
#


TOP	=	..

include ../common.mk


##########
#
# If you add directories other than one level deep
# you'll have to change the code below.
##########
LIBDIRS	= \
		./access \
		./class \
		./filters \
		./forms \
		./lp \
		./lpNet \
		./msgs \
		./oam \
		./bsd \
		./requests \
		./secure \
		./systems \
		./printers \
		./users


all install:
	for dir in $(LIBDIRS); \
	do \
		cd $$dir; \
		$(MAKE) DEBUG="$(DEBUG)" $@; \
		cd ..; \
	done

clean clobber:
	for dir in $(LIBDIRS); \
	do \
		cd $$dir; \
		$(MAKE) $@; \
		cd ..; \
	done

strip:

lint lintsrc lintlib:
	for dir in $(LIBDIRS); \
	do \
		cd $$dir; \
		$(MAKE) DEBUG="$(DEBUG)" FUNCDLC="$(FUNCDCL)" LINT="$(LINT)" $@; \
		cd ..; \
	done

libs:		all
