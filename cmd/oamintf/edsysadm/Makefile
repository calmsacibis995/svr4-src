#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:edsysadm/Makefile	1.10.2.2"

## default parameter definitions
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIB=$(ROOT)/lib
CFLAGS=-O
LDFLAGS=-s

## libraries used by this process
LIBADM=$(USRLIB)/libadm.a
LIBINTF=../libintf/libintf.a
LINTLIBS=\
	$(USRLIB)/llib-ladm.ln ../libintf/llib-lintf.ln

## options used to build this command
INCDIR=../intf_include
HDROPTS=-I $(INCDIR) -I $(INC)
LIBOPTS=-L ../libintf -lintf -ladm 

STRIP=strip
SIZE=size

MAINS=\
	delsysadm getintdef getfld findmenu updt_pkgdesc getpkgdesc
O_SHFILES=\
	edsysadm mkpkg mkmf mkpf mkchgs test_menu valfiles edhelp
O_DFILES=\
	Help \
	Form.chgmenu Form.chgtask Form.collision Form.getpkg Form.menu \
	Form.save Form.savepkg Form.select Form.task Form.test \
	Form.overwrite Form.redo\
	Menu.choices Menu.menu Menu.pkg Menu.task Menu.testmenu \
	Text.dups Text.ok Text.success Text.mkchgerr Text.tmerr \
	Text.gpderr Text.pkgerr

LINTFILES=\
	updt_pkgdesc getpkgdesc findmenu getintdef getfld delsysadm

.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all: $(O_SHFILES) $(MAINS)

getintdef: getintdef.o $(INCDIR)/intf.h
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

getfld: getfld.o
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

findmenu: findmenu.o $(INCDIR)/intf.h
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

delsysadm: delsysadm.o $(INCDIR)/intf.h $(INCDIR)/menu_io.h
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

getpkgdesc: getpkgdesc.o
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

updt_pkgdesc: updt_pkgdesc.o
	$(CC) -o $@ $(LDFLAGS) $@.o $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

clean:
	rm -f *.o

clobber: clean
	rm -f $(O_SHFILES) $(MAINS)

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

lintit:
	rm -f lint.out
	for file in $(LINTFILES) ;\
	do \
		lint -u -I $(INCDIR) -I $(INC) $$file.c $(LINTLIBS) >> lint.out ;\
	done

install: all xinstall

PROTO=Prototype
OBJECTS=$(MAINS) $(O_SHFILES) $(O_DFILES)
include ../Install
