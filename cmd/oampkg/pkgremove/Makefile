#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oampkg:pkgremove/Makefile	1.10.4.1"

SRCBASE=../
PROC=pkgremove
OBJECTS=$(PROC)

## default parameter definitions
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIB=$(ROOT)/lib
CFLAGS=-O 
LDFLAGS=-s

## libraries used by this process
LIBINST=../libinst/libinst.a
LINTLIBS=\
	$(USRLIB)/llib-lpkg.ln $(USRLIB)/llib-ladm.ln \
	../libinst/llib-linst.ln

## options used to build this command
HDROPTS=-I. -I ../hdrs -I $(INC)
LIBOPTS=-L ../libinst -linst -lpkg -ladm 

## objects which make up this process
OFILES=\
	main.o quit.o delmap.o check.o predepend.o

## C files to be linted
LINTFILES=\
	main.c quit.c delmap.c check.c predepend.c

## process build rules 
.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all:	$(PROC)

$(PROC): $(OFILES) $(LIBINST)
	$(CC) -o $(PROC) $(LDFLAGS) $(OFILES) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $(PROC)

clean:
	rm -f $(OFILES)

clobber: clean
	rm -f $(PROC)

lintit:
	lint $(HDROPTS) $(LINTFILES) $(LINTLIBS) >lint.out 2>&1

include $(SRCBASE)Install
