#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kdb:kdb/Makefile	1.4"

BUS = AT386
ARCH = AT386
MORECPP = -D$(BUS) -D$(ARCH)
DEFLIST =
FRC =

STRIP = strip
INCRT = ../..
CC = cc
DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
CONF = $(ROOT)/etc/conf
OFILE = $(CONF)/pack.d/kdb/Driver.o

FILES = db.o dbcon.o dbintrp.o dblex.o dbshow.o dbshow2.o \
		bits.o extn.o tbls.o utls.o opset.o


all:		$(OFILE)

$(OFILE):       $(FILES) $(FRC)
	$(LD) -r -o $@ $(FILES)

.c.o:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $*.c

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILE)

FRC:


#
# Header Dependencies
#

bits.o:	bits.c \
	dis.h \
	$(INCRT)/sys/types.h \
	$(FRC)

db.o:	db.c \
	debugger.h \
	../db_as.h \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysmsg.h \
	$(INCRT)/sys/xdebug.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

dbcon.o: dbcon.c \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(FRC)

dbintrp.o: dbintrp.c \
	debugger.h \
	dbcmd.h \
	../db_as.h \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/proc.h \
	$(FRC)

dblex.o: dblex.c \
	debugger.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(FRC)

dbshow.o: dbshow.c \
	debugger.h \
	../db_as.h \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fs/snode.h \
	$(INCRT)/sys/procfs.h \
	$(INCRT)/fs/proc/prdata.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

dbshow2.o: dbshow2.c \
	../db_as.h \
	$(INCRT)/sys/kdebugger.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/regset.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/fs/ufs_inode.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

extn.o:	extn.c \
	dis.h \
	structs.h \
	../db_as.h \
	$(INCRT)/sys/types.h \
	$(FRC)

opset.o: opset.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(FRC)

utls.o:	utls.c \
	dis.h \
	structs.h \
	../db_as.h \
	$(INCRT)/sys/types.h \
	$(FRC)
