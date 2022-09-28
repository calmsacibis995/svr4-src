#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)crash-3b2:crash.mk	1.11.14.1"

BUS = AT386
ARCH = AT386

STRIP = 
DBO = -DDBO
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"

TESTDIR = .
ROOT = 
INSDIR = $(ROOT)/usr/sbin
SBIN_INSDIR = $(ROOT)/sbin
INS = install
LDFLAGS = -s
INC=$(ROOT)/usr/include
SYMLINK = :
INCSRC=$(ROOT)/usr/src/uts/i386
COMFLAGS = -D_KMEMUSER -I${INC} -I. -Uvax -Di386 $(DBO) -D$(BUS) -D$(ARCH)
CFLAGS= $(COMFLAGS) -O
LIBELF = -lelf
FRC =

OFILES= async.o \
	base.o \
	buf.o \
	callout.o \
	class.o \
	dis.o \
	disp.o \
	events.o \
	i386.o \
	init.o \
	inode.o \
	kma.o \
	lck.o \
	main.o \
	map.o \
	page.o \
	prnode.o \
	proc.o \
	pty.o \
	resource.o \
	rfs.o \
	rt.o \
	search.o \
	size.o \
	sizenet.o \
	snode.o \
	stat.o \
	stream.o \
	symtab.o \
	ts.o \
	tty.o \
	u.o \
	ufs_inode.o \
	util.o \
	var.o \
	vfs.o \
	vfssw.o \
	vtop.o 

CFILES= async.c \
	base.c \
	buf.c \
	callout.c \
	class.c \
	dis.c \
	disp.c \
	events.c \
	i386.c \
	init.c \
	inode.c \
	kma.c \
	lck.c \
	main.c \
	map.c \
	page.c \
	prnode.c \
	proc.c \
	resource.c \
	rfs.c \
	rt.c \
	search.c \
	size.c \
	sizenet.c \
	snode.c \
	stat.c \
	stream.c \
	symtab.c \
	ts.c \
	tty.c \
	u.c \
	util.c \
	var.c \
	vfs.c \
	vfssw.c \
	vtop.c 


all: crash ldsysdump memsize

crash:	$(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/crash $(OFILES) $(LIBELF) $(SHLIBS)

ldsysdump: ldsysdump.sh
	cp ldsysdump.sh $(TESTDIR)/ldsysdump

memsize: memsize.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/memsize memsize.c $(ROOTLIBS)

install: ins_crash ins_ldsysdump ins_memsize

ins_crash: crash
	-rm -f $(ROOT)/etc/crash
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/crash
	-$(SYMLINK) /usr/sbin/crash $(ROOT)/etc/crash

ins_ldsysdump: ldsysdump
	-rm -f $(ROOT)/etc/ldsysdump
	$(INS) -f $(SBIN_INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/ldsysdump
	-$(SYMLINK) /sbin/ldsysdump $(ROOT)/etc/ldsysdump

ins_memsize: memsize
	-rm -f $(ROOT)/etc/memsize
	$(INS) -f $(SBIN_INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/memsize
	-$(SYMLINK) /sbin/memsize $(ROOT)/etc/memsize

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/crash
	-rm -f $(TESTDIR)/ldsysdump
	-rm -f $(TESTDIR)/memsize

xref: $(CFILES) $(HFILES) 
	cxref -c $(COMFLAGS) $(CFILES) | pr -h crash.cxref | opr

lint: $(CFILES) $(HFILES) 
	lint $(COMFLAGS) -uh $(CFILES) 

prall:
	pr -n $(CFILES) | opr
	pr -n $(HFILES) | opr

FRC:
