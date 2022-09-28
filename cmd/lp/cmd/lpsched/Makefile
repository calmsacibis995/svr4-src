#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:cmd/lpsched/Makefile	1.10.3.1"
#
# Makefile for LP scheduler trio
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-O

##########
#
# If you add directories other than one level deep
# you'll have to change the code below.
##########
CMDDIRS= \
		./lpNet \
		./lpsched


all:		cmds

install clean clobber:
	for dir in $(CMDDIRS); \
	do \
		cd $$dir; \
		$(MAKE) $@; \
		cd ..; \
	done

strip:
	for dir in $(CMDDIRS); \
	do \
		cd $$dir; \
		$(MAKE) STRIP=$(STRIP) strip; \
		cd ..; \
	done

cmds:
	for dir in $(CMDDIRS); \
	do \
		cd $$dir; \
		$(MAKE) DEBUG="$(DEBUG)"; \
		cd ..; \
	done

lint:
	for dir in $(CMDDIRS); \
	do \
		cd $$dir; \
		$(MAKE) DEBUG="$(DEBUG)" LINT="$(LINT)" lint; \
		cd ..; \
	done
