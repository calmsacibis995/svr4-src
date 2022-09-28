#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
# 	All Rights Reserved
#
#ident	"@(#)fmli:qued/qued.mk	1.16"
#

INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY=libqued.a
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE =  -I$(HEADER1) -I$(CURSES_H)
CFLAGS = -O
AR=	ar


$(LIBRARY): \
		$(LIBRARY)(acs_io.o) \
		$(LIBRARY)(arrows.o) \
		$(LIBRARY)(copyfield.o) \
		$(LIBRARY)(fclear.o) \
		$(LIBRARY)(fgo.o) \
		$(LIBRARY)(fstream.o) \
		$(LIBRARY)(fput.o) \
		$(LIBRARY)(fread.o) \
		$(LIBRARY)(initfield.o) \
		$(LIBRARY)(editmulti.o) \
		$(LIBRARY)(editsingle.o) \
		$(LIBRARY)(getfield.o) \
		$(LIBRARY)(mfuncs.o) \
		$(LIBRARY)(multiline.o) \
		$(LIBRARY)(putfield.o) \
		$(LIBRARY)(setfield.o) \
		$(LIBRARY)(scrollbuf.o) \
		$(LIBRARY)(sfuncs.o) \
		$(LIBRARY)(singleline.o) \
		$(LIBRARY)(vfuncs.o) \
		$(LIBRARY)(wrap.o) 

$(LIBRARY)(acs_io.o): $(HEADER1)/token.h
$(LIBRARY)(acs_io.o): $(HEADER1)/vt.h
$(LIBRARY)(acs_io.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(acs_io.o): $(HEADER1)/winp.h
$(LIBRARY)(acs_io.o): $(HEADER1)/wish.h
$(LIBRARY)(acs_io.o): ./fmacs.h
$(LIBRARY)(acs_io.o): acs_io.c

$(LIBRARY)(arrows.o): $(HEADER1)/ctl.h
$(LIBRARY)(arrows.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(arrows.o): $(HEADER1)/winp.h
$(LIBRARY)(arrows.o): $(HEADER1)/wish.h
$(LIBRARY)(arrows.o): ./fmacs.h
$(LIBRARY)(arrows.o): arrows.c

$(LIBRARY)(copyfield.o): $(HEADER1)/moremacros.h
$(LIBRARY)(copyfield.o): $(HEADER1)/terror.h
$(LIBRARY)(copyfield.o): $(HEADER1)/token.h
$(LIBRARY)(copyfield.o): $(HEADER1)/winp.h
$(LIBRARY)(copyfield.o): $(HEADER1)/wish.h
$(LIBRARY)(copyfield.o): ./fmacs.h
$(LIBRARY)(copyfield.o): copyfield.c

$(LIBRARY)(editmulti.o): $(HEADER1)/token.h
$(LIBRARY)(editmulti.o): $(HEADER1)/winp.h
$(LIBRARY)(editmulti.o): $(HEADER1)/wish.h
$(LIBRARY)(editmulti.o): ./fmacs.h
$(LIBRARY)(editmulti.o): editmulti.c

$(LIBRARY)(editsingle.o): $(HEADER1)/terror.h
$(LIBRARY)(editsingle.o): $(HEADER1)/token.h
$(LIBRARY)(editsingle.o): $(HEADER1)/winp.h
$(LIBRARY)(editsingle.o): $(HEADER1)/wish.h
$(LIBRARY)(editsingle.o): ./fmacs.h
$(LIBRARY)(editsingle.o): editsingle.c

$(LIBRARY)(editvirt.o): $(HEADER1)/token.h
$(LIBRARY)(editvirt.o): $(HEADER1)/winp.h
$(LIBRARY)(editvirt.o): $(HEADER1)/wish.h
$(LIBRARY)(editvirt.o): ./fmacs.h
$(LIBRARY)(editvirt.o): editvirt.c

$(LIBRARY)(fclear.o): $(HEADER1)/token.h
$(LIBRARY)(fclear.o): $(HEADER1)/winp.h
$(LIBRARY)(fclear.o): ./fmacs.h
$(LIBRARY)(fclear.o): fclear.c

$(LIBRARY)(fgo.o): $(HEADER1)/token.h
$(LIBRARY)(fgo.o): $(HEADER1)/winp.h
$(LIBRARY)(fgo.o): fgo.c

$(LIBRARY)(fput.o): $(HEADER1)/attrs.h
$(LIBRARY)(fput.o): $(HEADER1)/token.h
$(LIBRARY)(fput.o): $(HEADER1)/winp.h
$(LIBRARY)(fput.o): $(HEADER1)/wish.h
$(LIBRARY)(fput.o): ./fmacs.h
$(LIBRARY)(fput.o): fput.c

$(LIBRARY)(fread.o): $(HEADER1)/token.h
$(LIBRARY)(fread.o): $(HEADER1)/vt.h
$(LIBRARY)(fread.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fread.o): $(HEADER1)/winp.h
$(LIBRARY)(fread.o): $(HEADER1)/wish.h
$(LIBRARY)(fread.o): ./fmacs.h
$(LIBRARY)(fread.o): fread.c

$(LIBRARY)(fstream.o): $(HEADER1)/attrs.h
$(LIBRARY)(fstream.o): $(HEADER1)/ctl.h
$(LIBRARY)(fstream.o): $(HEADER1)/terror.h
$(LIBRARY)(fstream.o): $(HEADER1)/token.h
$(LIBRARY)(fstream.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fstream.o): $(HEADER1)/winp.h
$(LIBRARY)(fstream.o): $(HEADER1)/wish.h
$(LIBRARY)(fstream.o): ./fmacs.h
$(LIBRARY)(fstream.o): fstream.c

$(LIBRARY)(getfield.o): $(HEADER1)/terror.h
$(LIBRARY)(getfield.o): $(HEADER1)/token.h
$(LIBRARY)(getfield.o): $(HEADER1)/winp.h
$(LIBRARY)(getfield.o): $(HEADER1)/wish.h
$(LIBRARY)(getfield.o): ./fmacs.h
$(LIBRARY)(getfield.o): getfield.c

$(LIBRARY)(initfield.o): $(HEADER1)/attrs.h
$(LIBRARY)(initfield.o): $(HEADER1)/ctl.h
$(LIBRARY)(initfield.o): $(HEADER1)/terror.h
$(LIBRARY)(initfield.o): $(HEADER1)/token.h
$(LIBRARY)(initfield.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(initfield.o): $(HEADER1)/winp.h
$(LIBRARY)(initfield.o): $(HEADER1)/wish.h
$(LIBRARY)(initfield.o): ./fmacs.h
$(LIBRARY)(initfield.o): initfield.c

$(LIBRARY)(mfuncs.o): $(HEADER1)/vt.h
$(LIBRARY)(mfuncs.o): $(HEADER1)/winp.h
$(LIBRARY)(mfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(mfuncs.o): ./fmacs.h
$(LIBRARY)(mfuncs.o): mfuncs.c

$(LIBRARY)(multiline.o): $(HEADER1)/token.h
$(LIBRARY)(multiline.o): $(HEADER1)/winp.h
$(LIBRARY)(multiline.o): $(HEADER1)/wish.h
$(LIBRARY)(multiline.o): ./fmacs.h
$(LIBRARY)(multiline.o): multiline.c

$(LIBRARY)(putfield.o): $(HEADER1)/moremacros.h
$(LIBRARY)(putfield.o): $(HEADER1)/terror.h
$(LIBRARY)(putfield.o): $(HEADER1)/token.h
$(LIBRARY)(putfield.o): $(HEADER1)/winp.h
$(LIBRARY)(putfield.o): $(HEADER1)/wish.h
$(LIBRARY)(putfield.o): ./fmacs.h
$(LIBRARY)(putfield.o): putfield.c

$(LIBRARY)(scrollbuf.o): $(HEADER1)/terror.h
$(LIBRARY)(scrollbuf.o): $(HEADER1)/token.h
$(LIBRARY)(scrollbuf.o): $(HEADER1)/winp.h
$(LIBRARY)(scrollbuf.o): $(HEADER1)/wish.h
$(LIBRARY)(scrollbuf.o): ./fmacs.h
$(LIBRARY)(scrollbuf.o): scrollbuf.c

$(LIBRARY)(setfield.o): $(HEADER1)/attrs.h
$(LIBRARY)(setfield.o): $(HEADER1)/terror.h
$(LIBRARY)(setfield.o): $(HEADER1)/token.h
$(LIBRARY)(setfield.o): $(HEADER1)/winp.h
$(LIBRARY)(setfield.o): ./fmacs.h
$(LIBRARY)(setfield.o): setfield.c

$(LIBRARY)(sfuncs.o): $(HEADER1)/attrs.h
$(LIBRARY)(sfuncs.o): $(HEADER1)/token.h
$(LIBRARY)(sfuncs.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(sfuncs.o): $(HEADER1)/winp.h
$(LIBRARY)(sfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(sfuncs.o): ./fmacs.h
$(LIBRARY)(sfuncs.o): sfuncs.c

$(LIBRARY)(singleline.o): $(HEADER1)/token.h
$(LIBRARY)(singleline.o): $(HEADER1)/winp.h
$(LIBRARY)(singleline.o): $(HEADER1)/wish.h
$(LIBRARY)(singleline.o): ./fmacs.h
$(LIBRARY)(singleline.o): singleline.c

$(LIBRARY)(syncfield.o): $(HEADER1)/token.h
$(LIBRARY)(syncfield.o): $(HEADER1)/winp.h
$(LIBRARY)(syncfield.o): ./fmacs.h
$(LIBRARY)(syncfield.o): syncfield.c

$(LIBRARY)(vfuncs.o): $(HEADER1)/ctl.h
$(LIBRARY)(vfuncs.o): $(HEADER1)/terror.h
$(LIBRARY)(vfuncs.o): $(HEADER1)/token.h
$(LIBRARY)(vfuncs.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vfuncs.o): $(HEADER1)/winp.h
$(LIBRARY)(vfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(vfuncs.o): ./fmacs.h
$(LIBRARY)(vfuncs.o): vfuncs.c

$(LIBRARY)(wrap.o): $(HEADER1)/terror.h
$(LIBRARY)(wrap.o): $(HEADER1)/token.h
$(LIBRARY)(wrap.o): $(HEADER1)/winp.h
$(LIBRARY)(wrap.o): $(HEADER1)/wish.h
$(LIBRARY)(wrap.o): ./fmacs.h
$(LIBRARY)(wrap.o): wrap.c

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o


###### Standard makefile targets #####

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	libqued.a 
