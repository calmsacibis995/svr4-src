#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oampkg:pkgmk/pkgmk.mk	1.8.5.1"

SRCBASE=../
PROC=pkgmk
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
HDROPTS=-I ../hdrs -I $(INC)
LIBOPTS=-L ../libinst -linst -lpkg -ladm 
LIBOPTS3.2= -L ../libinst -linst3.2 -L $(ROOT)/usr/lib -lpkg3.2 -ladm3.2

## objects which make up this process
OFILES=\
	splpkgmap.o main.o quit.o mkpkgmap.o

## C files to be linted
LINTFILES=\
	splpkgmap.c main.c quit.c mkpkgmap.c

## process build rules 
.c.o:
	$(CC) -c $(HDROPTS) $(PRESVR4) $(CFLAGS) $<

all:	$(PROC) $(ROOT)/xenv/$(PROC)

$(PROC): $(OFILES) $(LIBINST)
	$(CC) -o $(PROC) $(LDFLAGS) $(OFILES) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $(PROC)

$(ROOT)/xenv/$(PROC):
	- if [ -d $(ROOT)/xenv ]; then cc -I/usr/include $(HDROPTS) -O -s $(LINTFILES) -D PRESVR4 -D _STYPES -U __STDC__ -o $(ROOT)/xenv/$(PROC) $(LIBOPTS3.2); fi

clean:
	rm -f $(OFILES)

clobber: clean
	rm -f $(PROC)

lintit:
	lint $(HDROPTS) $(LINTFILES) $(LINTLIBS) >lint.out 2>&1

include $(SRCBASE)Install
