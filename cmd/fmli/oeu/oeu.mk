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
#ident	"@(#)fmli:oeu/oeu.mk	1.11"
#

INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY=liboeu.a
HEADER1=../inc
INCLUDE=	-I$(HEADER1)
DEFS=	-DJUSTCHECK

AR=		ar
CFLAGS=		-O


$(LIBRARY): \
		$(LIBRARY)(oeu.o) \
		$(LIBRARY)(oeucheck.o) \
		$(LIBRARY)(rm_atob.o) \
		$(LIBRARY)(genparse.o)

$(LIBRARY)(genparse.o): $(HEADER1)/io.h
$(LIBRARY)(genparse.o): $(HEADER1)/mail.h
$(LIBRARY)(genparse.o): $(HEADER1)/mess.h
$(LIBRARY)(genparse.o): $(HEADER1)/mio.h
$(LIBRARY)(genparse.o): $(HEADER1)/moremacros.h
$(LIBRARY)(genparse.o): $(HEADER1)/parse.h
$(LIBRARY)(genparse.o): $(HEADER1)/retcds.h
$(LIBRARY)(genparse.o): $(HEADER1)/smdef.h
$(LIBRARY)(genparse.o): $(HEADER1)/terror.h
$(LIBRARY)(genparse.o): $(HEADER1)/typetab.h
$(LIBRARY)(genparse.o): $(HEADER1)/wish.h
$(LIBRARY)(genparse.o): genparse.c

$(LIBRARY)(oeu.o): $(HEADER1)/io.h
$(LIBRARY)(oeu.o): $(HEADER1)/mail.h
$(LIBRARY)(oeu.o): $(HEADER1)/mess.h
$(LIBRARY)(oeu.o): $(HEADER1)/mio.h
$(LIBRARY)(oeu.o): $(HEADER1)/parse.h
$(LIBRARY)(oeu.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(oeu.o): $(HEADER1)/retcds.h
$(LIBRARY)(oeu.o): $(HEADER1)/smdef.h
$(LIBRARY)(oeu.o): $(HEADER1)/sizes.h
$(LIBRARY)(oeu.o): $(HEADER1)/terror.h
$(LIBRARY)(oeu.o): $(HEADER1)/typetab.h
$(LIBRARY)(oeu.o): $(HEADER1)/wish.h
$(LIBRARY)(oeu.o): oeu.c

$(LIBRARY)(oeucheck.o): $(HEADER1)/io.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/mail.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/mess.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/mio.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/parse.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/retcds.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/smdef.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/terror.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/typetab.h
$(LIBRARY)(oeucheck.o): $(HEADER1)/wish.h
$(LIBRARY)(oeucheck.o): oeu.c
	/bin/cp oeu.c oeucheck.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(DEFS) oeucheck.c
	$(AR) rv $@ oeucheck.o
	/bin/rm -f oeucheck.o
	/bin/rm -f oeucheck.c


$(LIBRARY)(rm_atob.o): rm_atob.c

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets ######
all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
