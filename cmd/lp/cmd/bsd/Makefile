#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:cmd/bsd/Makefile	1.7.3.1"


TOP	=	../..

include ../../common.mk


DEBUG	=


CMDS	= \
		lpc \
		lpq \
		lpr \
		lprm \
		lptest


all install clean clobber strip:
	for d in $(CMDS); \
	do \
		cd $$d; \
		$(MAKE) DEBUG="$(DEBUG)" $@; \
		cd ..; \
	done
