#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lprof:i386/makefile	1.3"

LPROFBASE	= `pwd | sed 's/lprof.*/lprof/'`

SGSBASE		= $(LPROFBASE)/..
CMDBASE		= $(LPROFBASE)/../..

include ../lprofinc.mk

all install lintit clean clobber:
	cd ..; $(MAKE) -f lprof.mk $@ $(ENVPARMS)

