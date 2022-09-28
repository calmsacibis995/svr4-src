#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#
#ident	"@(#)fmli:xx/xx.mk	1.36"
#

INC=$(ROOT)/usr/include
USR=$(ROOT)/usr
#USRLIB=$(CCSROOT)/usr/ccs/lib
CURSES_H =$(INC)
LIBWISH	=	../wish/libwish.a
LIBVT	=	../vt/libvt.a
LIBFORM	=	../form/libform.a
LIBMENU	=	../menu/libmenu.a
LIBQUED	=	../qued/libqued.a
LIBOH	=	../oh/liboh.a
LIBOEU	=	../oeu/liboeu.a
LIBPROC	=	../proc/libproc.a
LIBSYS	=	../sys/libsys.a
#LIBCURSES =	$(USRLIB)/libcurses.a

MKDIR	=	mkdir
BIN = $(USR)/bin
HEADER1=../inc
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)

#LIBS = $(LIBWISH) $(LIBOH) $(LIBOEU) $(LIBFORM) $(LIBQUED) $(LIBMENU) $(LIBPROC) $(LIBVT) $(LIBSYS) $(LIBCURSES)

LIBS = $(LIBWISH) $(LIBOH) $(LIBOEU) $(LIBFORM) $(LIBQUED) $(LIBMENU) $(LIBPROC) $(LIBVT) $(LIBSYS) 

CFLAGS= -O 

LDFLAGS =  -s

BCMDS =	fmli vsig 

CMDS = $(BCMDS) 

fmli: main.o $(LIBS)
	$(CC) $(LDFLAGS) -o $@ main.o $(LIBS) -lgen -lcurses $(SHLIBS)

main.o: $(HEADER1)/actrec.h
main.o: $(HEADER1)/ctl.h
main.o: $(HEADER1)/moremacros.h
main.o: $(HEADER1)/sizes.h
main.o: $(HEADER1)/terror.h
main.o: $(HEADER1)/token.h
main.o: $(HEADER1)/vtdefs.h
main.o: $(HEADER1)/wish.h
main.o: main.c

vsig: vsig.o
	$(CC) $(LDFLAGS) -o $@ vsig.o

vsig.o: $(HEADER1)/sizes.h
vsig.o: vsig.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

###### Standard Makefile Targets ######

all:	$(CMDS)

install: $(BIN)
	@set +e;\
	for f in $(BCMDS);\
	do\
		install -f $(BIN) $$f;\
		$(CH) chgrp bin $(BIN)/$$f;\
		chmod 755 $(BIN)/$$f;\
		$(CH) chown bin $(BIN)/$$f;\
	done

clean: 
	@set +e;\
	for f in $(BCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	/bin/rm -f *.o

clobber:	clean
