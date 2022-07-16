#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#ident	"@(#)sti:form/makefile	1.15"
#

ROOT=
OWN=bin
GRP=bin

PWD=.
INC=		$(ROOT)/usr/include
USRLIB=		$(ROOT)/usr/lib
CCSLIB=		$(ROOT)/usr/ccs/lib
INCLUDE=	-I $(PWD) -I $(PWD)/.. -I$(INC)

LIBRARY=	libform.a

PRODUCT= 	$(LIBRARY) llib-lform.ln llib-lform

CC=		cc
AR=		ar
CFLAGS=		-O
LINT=		lint
LINTFLAGS=-u
PR=		pr
LP=		lp -omode=h


OBJ=	$(LIBRARY)(form.o)		\
	$(LIBRARY)(form_win.o)	\
	$(LIBRARY)(form_sub.o)	\
	$(LIBRARY)(form_init.o)	\
	$(LIBRARY)(form_term.o)	\
	$(LIBRARY)(field_init.o)	\
	$(LIBRARY)(field_term.o)	\
	$(LIBRARY)(form_user.o)	\
	$(LIBRARY)(form_opts.o)	\
	$(LIBRARY)(field.o)		\
	$(LIBRARY)(field_just.o)	\
	$(LIBRARY)(field_fore.o)	\
	$(LIBRARY)(field_back.o)	\
	$(LIBRARY)(field_pad.o)	\
	$(LIBRARY)(field_buf.o)	\
	$(LIBRARY)(field_stat.o)	\
	$(LIBRARY)(field_user.o)	\
	$(LIBRARY)(field_opts.o)	\
	$(LIBRARY)(driver.o)	\
	$(LIBRARY)(chg_field.o)	\
	$(LIBRARY)(chg_page.o)	\
	$(LIBRARY)(chg_char.o)	\
	$(LIBRARY)(chg_data.o)	\
	$(LIBRARY)(post.o)		\
	$(LIBRARY)(utility.o)	\
	$(LIBRARY)(fieldtype.o)	\
	$(LIBRARY)(ty_enum.o)	\
	$(LIBRARY)(ty_int.o)	\
	$(LIBRARY)(ty_num.o)	\
	$(LIBRARY)(ty_regexp.o)	\
	$(LIBRARY)(ty_alpha.o)	\
	$(LIBRARY)(ty_alnum.o)	\
	$(LIBRARY)(regcmp.o)	\
	$(LIBRARY)(regex.o)

SOURCES=form.c			\
	form_win.c		\
	form_sub.c		\
	form_init.c		\
	form_term.c		\
	field_init.c		\
	field_term.c		\
	form_user.c		\
	form_opts.c		\
	field.c			\
	field_just.c		\
	field_fore.c		\
	field_back.c		\
	field_pad.c		\
	field_buf.c		\
	field_stat.c		\
	field_user.c		\
	field_opts.c		\
	driver.c		\
	chg_field.c		\
	chg_page.c		\
	chg_char.c		\
	chg_data.c		\
	post.c			\
	utility.c		\
	fieldtype.c		\
	ty_enum.c		\
	ty_int.c		\
	ty_num.c		\
	ty_regexp.c		\
	ty_alpha.c		\
	ty_alnum.c		\
	regcmp.c		\
	regex.c

HFILES=$(PWD)/../eti.h form.h utility.h

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	rm -f $*.o

### Standard Makefile targets ###

all:		$(PRODUCT)

$(LIBRARY):	$(OBJ)

$(OBJ):		$(HFILES)

llib-lform:	llib-lform.c
		ln llib-lform.c llib-lform

llib-lform.ln:	llib-lform.c
		$(LINT) -o form -vx $(INCLUDE) llib-lform.c

install:	all
	install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) $(LIBRARY)
	install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lform
	install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lform.ln
	install -f $(INC) -m 444 -u $(OWN) -g $(GRP) form.h

lintit:		
		$(LINT) $(INCLUDE) $(LINTFLAGS) -l curses $(SOURCES)

clean:
		rm -f *.o

clobber:	clean
		rm -f $(PRODUCT)

print:
		$(PR) $(SOURCES) $(HFILES) | $(LP)

.PRECIOUS:	$(LIBRARY)
