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
#ident	"@(#)fmli:form/form.mk	1.7"
#




INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY=libform.a
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)
AR=		ar
CFLAGS=		-O


$(LIBRARY): \
		$(LIBRARY)(fcheck.o) \
		$(LIBRARY)(fclose.o) \
		$(LIBRARY)(fctl.o) \
		$(LIBRARY)(fcurrent.o) \
		$(LIBRARY)(fcustom.o) \
		$(LIBRARY)(fdefault.o) \
		$(LIBRARY)(frefresh.o)

$(LIBRARY)(fcheck.o): $(HEADER1)/form.h
$(LIBRARY)(fcheck.o): $(HEADER1)/token.h
$(LIBRARY)(fcheck.o): $(HEADER1)/winp.h
$(LIBRARY)(fcheck.o): $(HEADER1)/wish.h
$(LIBRARY)(fcheck.o): fcheck.c

$(LIBRARY)(fclose.o): $(HEADER1)/form.h
$(LIBRARY)(fclose.o): $(HEADER1)/token.h
$(LIBRARY)(fclose.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(fclose.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fclose.o): $(HEADER1)/winp.h
$(LIBRARY)(fclose.o): $(HEADER1)/wish.h
$(LIBRARY)(fclose.o): fclose.c

$(LIBRARY)(fctl.o): $(HEADER1)/ctl.h
$(LIBRARY)(fctl.o): $(HEADER1)/form.h
$(LIBRARY)(fctl.o): $(HEADER1)/token.h
$(LIBRARY)(fctl.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fctl.o): $(HEADER1)/winp.h
$(LIBRARY)(fctl.o): $(HEADER1)/wish.h
$(LIBRARY)(fctl.o): fctl.c

$(LIBRARY)(fcurrent.o): $(HEADER1)/form.h
$(LIBRARY)(fcurrent.o): $(HEADER1)/token.h
$(LIBRARY)(fcurrent.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fcurrent.o): $(HEADER1)/winp.h
$(LIBRARY)(fcurrent.o): $(HEADER1)/wish.h
$(LIBRARY)(fcurrent.o): fcurrent.c

$(LIBRARY)(fcustom.o): $(HEADER1)/form.h
$(LIBRARY)(fcustom.o): $(HEADER1)/token.h
$(LIBRARY)(fcustom.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(fcustom.o): $(HEADER1)/winp.h
$(LIBRARY)(fcustom.o): $(HEADER1)/wish.h
$(LIBRARY)(fcustom.o): fcustom.c

$(LIBRARY)(fdefault.o): $(HEADER1)/ctl.h
$(LIBRARY)(fdefault.o): $(HEADER1)/form.h
$(LIBRARY)(fdefault.o): $(HEADER1)/terror.h
$(LIBRARY)(fdefault.o): $(HEADER1)/token.h
$(LIBRARY)(fdefault.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fdefault.o): $(HEADER1)/winp.h
$(LIBRARY)(fdefault.o): $(HEADER1)/wish.h
$(LIBRARY)(fdefault.o): fdefault.c

$(LIBRARY)(frefresh.o): $(HEADER1)/attrs.h
$(LIBRARY)(frefresh.o): $(HEADER1)/form.h
$(LIBRARY)(frefresh.o): $(HEADER1)/token.h
$(LIBRARY)(frefresh.o): $(HEADER1)/winp.h
$(LIBRARY)(frefresh.o): $(HEADER1)/wish.h
$(LIBRARY)(frefresh.o): frefresh.c

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

##### Standard makefile targets ######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
