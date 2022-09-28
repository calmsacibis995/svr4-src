#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:intf_install/Makefile	1.6.2.2"

## default parameter definitions
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIB=$(ROOT)/lib
CFLAGS=-O 
LDFLAGS=-s

## libraries used by this process
LIBINTF=../libintf/libintf.a
LINTLIBS=\
	../libintf/llib-lintf.ln

#	$(USRLIB)/llib-ladm.ln ../libintf/llib-lintf.ln

## options used to build this command
INCDIR=../intf_include
HDROPTS=-I $(INCDIR) -I $(INC)
LIBOPTS=-L ../libintf -lintf -ladm 

STRIP=strip
SIZE=size

PROC=mod_menus
O_CFILES=\
	mod_menus.o
O_SHFILES=\
	intf_inst
O_DFILES=\
	i.OAMmif

## objects which make up this process
LINTFILES=\
	mod_menus.c

.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all: $(O_SHFILES) $(O_DFILES) $(PROC)

$(PROC): $(O_CFILES)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

mod_menus.o: $(INCDIR)/intf.h $(INCDIR)/menu_io.h $(INCDIR)/inst_err.h

clean:
	rm -f *.o

clobber: clean
	rm -f $(MAINS) $(O_SHFILES) $(O_HELPFILES) mod_menus

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

lintit:
	rm -f lint.out
	lint -I $(INCDIR) -I $(INC) $(LINTFILES) $(LINTLIBS) >>lint.out

install: all xinstall

PROTO=Prototype
OBJECTS=$(PROC) $(O_SHFILES) $(O_DFILES) $(O_HELPFILES)
include ../Install
