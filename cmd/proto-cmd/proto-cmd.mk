#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto-cmd:proto-cmd.mk	1.3"

#       Makefile for Packaging and Installation Commands

ROOTLIBS=-dn
INC=$(ROOT)/usr/include
INSDIR=$(ROOT)/etc
INSDIR_NF=$(ROOT)/usr/bin
USBIN=$(ROOT)/usr/sbin
ARCH=AT386
BUS=AT386

ALL=machine_type links setmods setmods.elf mkflist contents x286

NOT_FOUND=bdiff.nf adv.nf 300.nf crypt.nf nlsadmin.nf fsba.nf

all: $(ALL) pkginfo default

install: all $(ROOT)/var/sadm/pkg/dfm $(ROOT)/tmp
	install -f $(USBIN) machine_type
	install -f $(INSDIR) links
	install -f $(INSDIR) setmods
	install -f $(INSDIR) setmods.elf
	install -f $(INSDIR) mkflist
	install -f $(INSDIR) contents
	install -f $(ROOT)/usr/bin x286
	cp pkginfo $(ROOT)/var/sadm/pkg/dfm/pkginfo
	cp default $(ROOT)/tmp/default

$(ROOT)/var/sadm/pkg/dfm:
	-mkdir $(ROOT)/var/sadm/pkg/dfm

$(ROOT)/tmp:
	-mkdir $(ROOT)/tmp

fixswap: fixswap.c
	cc -s $(CFLAGS) -o $@ $?
	echo fixswap | sh ../../proto/i386/prep NO_UNIX

machine_type: machine_type.c
	$(CC) -O -s -o $@ -D$(BUS) -D$(ARCH) -I$(INC) $? $(ROOTLIBS)

links: links.c
	$(CC) -s $(CFLAGS) -o $@ $? $(SHLIBS)

x286: x286.c
	$(CC) -s $(CFLAGS) -o $@ $? $(SHLIBS)

setmods contents mkflist:
	cc -s $(CFLAGS) -o $@ $@.c

setmods.elf: setmods.c
	$(CC) -s $(CFLAGS) -o $@ $? $(SHLIBS)

clean:
	rm -f *.o

clobber: clean
	rm -f $(ALL)
