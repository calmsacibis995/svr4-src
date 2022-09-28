#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)fmli:makefile	1.8"
#


INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
#LIBCURSES=$(USRLIB)/libcurses.a
CURSES_H=$(INC)
CFLAGS= -O
LDFLAGS= -s

DIRS =	form menu oeu oh proc qued sys vt wish xx 



all:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		make -f $$d.mk INC="$(INC)" CURSES_H="$(CURSES_H)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $@;\
		cd ..;\
	done;\
	echo 'fmli.mk: finished making target "all"'

install: all
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		make -f $$d.mk INC="$(INC)" CURSES_H="$(CURSES_H)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $@;\
		cd ..;\
	done;\
	echo 'fmli.mk: finished making target "install"'


clean:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clean $@;\
		cd ..;\
	done;\
	echo 'fmli.mk: finished making target "clean"'

clobber: 
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		make -f $$d.mk clobber $@;\
		cd ..;\
	done;\
	echo 'fmli.mk: finished making target "clobber"'
