#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:interface/Makefile	1.14.2.2"

## default parameter definitions
INC=$(ROOT)/usr/include
INCDIR=../intf_include
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
CFLAGS=-O 
LDFLAGS=-s
STRIP=strip
SIZE=size

## libraries used by this process
LIBINTF=../libintf/libintf.a
LINTLIBS=\
	$(USRLIB)/llib-ladm.ln ../libintf/llib-lintf.ln

## options used to build this command
HDROPTS=-I $(INCDIR) -I $(INC)
LIBOPTS=-L ../libintf -lintf -ladm 

MAINS=object_gen sysadm ie_build itemhelp
ILINKS=setup powerdown checkfsys makefsys mountfsys umountfsys
O_DFILES=\
	Text.oamhelp main.menu express oam.init resrv.fs \
        oam.cmd Form.sysadm Menu.sysadm Text.sysadm  \
        Form.interface Menu.interface Text.interface Text.itemhelp
O_MSGFILES=\
	contmsg reptmsg form.msg display.msg conf.msg nodev.msg \
	trept.msg frept.msg ssmenu.msg msmenu.msg resrv.fs

O_CFILES1=get_desc.o interface.o
C_CFILES1=get_desc.c interface.c

C_CFILES2=sysadm.c print_err.c
O_CFILES2=sysadm.o print_err.o

C_CFILES3=ie_build.c print_err.c
O_CFILES3=ie_build.o print_err.o

C_CFILES4=itemhelp.c
O_CFILES4=itemhelp.o

#LINTFILES=get_desc.c interface.c sysadm.c print_err.c ie_build.c
LINTFILES=ie_build.c

all: $(MAINS) $(O_DFILES) $(O_MSGFILES)

$(O_MSGFILES):
	grep -v "^#ident" id.$(@) >$(@)

.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

object_gen: $(O_CFILES1) $(LIBINTF)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES1) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

sysadm: $(O_CFILES2) $(LIBINTF)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES2) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@
	ln $@ setup
	ln $@ powerdown
	ln $@ checkfsys
	ln $@ makefsys
	ln $@ mountfsys
	ln $@ umountfsys

ie_build: $(O_CFILES3) $(LIBINTF)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES3) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

itemhelp: $(O_CFILES4) $(LIBINTF)
	$(CC) -o $@ $(LDFLAGS) $(O_CFILES4) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $@

interface.o:	local.h $(INCDIR)/intf.h
get_desc.o:	local.h
sysadm.o:	print_err.h errors.h sysadm.h $(INCDIR)/intf.h
print_err.o:	print_err.h
ie_build.o:		$(INCDIR)/intf.h

clean:
	rm -f $(O_CFILES1) $(O_CFILES2) $(O_CFILES3) $(O_CFILES4)

clobber: clean
	rm -f $(MAINS) $(ILINKS) $(O_MSGFILES)

size: all
	$(SIZE) $(MAINS)

strip: all
	$(SIZE) $(MAINS)

lintit:
	rm -f lint.out
	for file in $(LINTFILES) ;\
	do \
		echo Begin lint for $$file >> lint.out ;\
		echo >> lint.out ;\
		lint -I $(INCDIR) -I $(INC) $$file $(LINTLIBS) >>lint.out ;\
		echo >> lint.out ;\
		echo End lint for $$file >> lint.out ;\
	done

install: all xinstall

PROTO=Prototype
OBJECTS=$(MAINS) $(ILINKS) $(O_DFILES) $(O_MSGFILES)
include ../Install
