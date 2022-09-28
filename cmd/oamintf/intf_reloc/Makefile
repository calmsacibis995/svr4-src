#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:intf_reloc/Makefile	1.6.2.2"

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

MAINS=reloc_menus
O_CFILES=reloc.o oldmenu.o
O_SHFILES=intf_reloc
O_DFILES=
LINTFILES=reloc.c oldmenu.c

.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all: $(MAINS) $(O_SHFILES) $(O_HELPFILES)

reloc_menus: $(O_CFILES)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES) $(LDLIBPATH) $(LIBOPTS) 
	chmod 775 $@

reloc.o: $(INCDIR)/intf.h $(INCDIR)/inst_err.h $(INCDIR)/menu_io.h ./oldmenu.h
oldmenu.o: $(INCDIR)/intf.h ./oldmenu.h

clean:
	rm -f $(O_CFILES)

clobber: clean
	rm -f $(MAINS) $(O_SHFILES) $(O_HELPFILES)

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

lintit:
	rm -f lint.out
	for file in $(LINTFILES) ;\
	do \
		lint -u -I $(INCDIR) -I $(INC) $$file.c $(LINTLIBS) >>lint.out ;\
	done

install: all xinstall

PROTO=Prototype
OBJECTS=$(MAINS) $(O_SHFILES) $(O_DFILES)
include ../Install
