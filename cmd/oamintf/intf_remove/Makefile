#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:intf_remove/Makefile	1.5.2.2"

## default parameter definitions
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIB=$(ROOT)/lib
CFLAGS=-O 
LDFLAGS=-s

## libraries used by this process
LIBINTF=../libintf/libintf.a
LINTLIBS=\
	$(USRLIB)/llib-ladm.ln ../libintf/llib-lintf.ln

## options used to build this command
INCDIR=../intf_include
HDROPTS=-I $(INCDIR) -I $(INC)
LIBOPTS=-L ../libintf -lintf -ladm 

STRIP=strip
SIZE=size

MAINS=intf_remove
O_CFILES=remove.o rm_err.o
LINTFILES=remove.c rm_err.c

O_SHFILES=
O_DFILES=r.OAMintf

.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all: $(MAINS) $(O_SHFILES) $(O_HELPFILES)

intf_remove: $(O_CFILES)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES) $(LDLIBPATH) $(LIBOPTS) 
	chmod 775 $@

reloc.o: $(INCDIR)/intf.h rm_err.h
rm_err.o: rm_err.h

clean:
	rm -f $(O_CFILES)

clobber: clean
	rm -f $(MAINS) $(O_SHFILES) $(O_HELPFILES)

lintit:
	rm -f lint.out
	for file in $(LINTFILES) ;\
	do \
		lint -u -I $(INCDIR) -I $(INC) $$file.c $(LINTLIBS) >>lint.out ;\
	done


size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

install: all xinstall

PROTO=Prototype
OBJECTS=$(MAINS) $(O_SHFILES) $(O_DFILES) $(O_HELPFILES)
include ../Install
