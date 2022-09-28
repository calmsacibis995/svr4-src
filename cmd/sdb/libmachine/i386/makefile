#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:libmachine/i386/makefile	1.2"

TARGET = $(PRODLIB)/libmachine.a

include ../../util/std.make

ENVPARMS = PRODDIR="$(PRODDIR)" MACH="$(MACH)" \
    ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
    CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
    LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
    INSDIR="$(INSDIR)" STRIP="$(STRIP)" \
    AR="$(AR)" COMINC="$(COMINC)" MACHINC="$(MACHINC)" \
    INC="$(INC)" MAKE="$(MAKE)" YACC="$(YACC)" CPLUS="$(CPLUS)"

CCSOURCES = \
	Instr.C	\
	Tables.C

include DEPEND

include ../../util/lib.make
