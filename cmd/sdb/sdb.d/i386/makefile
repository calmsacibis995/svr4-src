#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:sdb.d/i386/makefile	1.2"

BASENAME = sdb

TARGET = ../../$(BASENAME)

include ../../util/std.make

CCSOURCES = \
	$(COMMON)/CCmain.C \
	$(COMMON)/do_assoc.C \
	$(COMMON)/parser.Y \
	$(COMMON)/srclines.C

CSOURCES = \
	$(COMMON)/main.c \
	$(COMMON)/scanner.l

$(TARGET):	$(PRODLIB)/libutil.a   $(PRODLIB)/libexecon.a
$(TARGET):	$(PRODLIB)/libexp.a    $(PRODLIB)/libmachine.a
$(TARGET):	$(PRODLIB)/libsymbol.a $(PRODLIB)/libint.a
$(TARGET):	$(PRODLIB)/libdbgen.a  $(PRODLIB)/libC.a

LIBRARIES = \
	$(PRODLIB)/libutil.a \
	$(PRODLIB)/libexecon.a \
	$(PRODLIB)/libexp.a \
	$(PRODLIB)/libmachine.a \
	$(PRODLIB)/libsymbol.a \
	$(PRODLIB)/libint.a \
	$(PRODLIB)/libdbgen.a \
	-lelf \
	-ll

include DEPEND

include ../../util/prog.make
