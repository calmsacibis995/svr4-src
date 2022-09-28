#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:libexp/i386/makefile	1.4"

TARGET = $(PRODLIB)/libexp.a

include ../../util/std.make

CCSOURCES = \
	Call.C	\
	Call2.C \
	fetch.c	\
	$(COMMON)/Expr.C \
	$(COMMON)/Label.C \
	$(COMMON)/Place.C \
	$(COMMON)/Rvalue.C \
	$(COMMON)/SDBinfo.C \
	$(COMMON)/format.C

include DEPEND

include ../../util/lib.make
