#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)eisa:add-on/scsi.in/io/scsi/Makefile	1.3"

ROOT =
MORECPP = -DAT386 -D_SYSTEMENV=4
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL $(MORECPP)
FRC =


DRIVER = Driver.o

all:	$(DRIVER)

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/scsi ] || mkdir ../../pkg/scsi
	cp $(DRIVER) ../../pkg/scsi/
	cp esc.h ../../pkg/sys/

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f esc.o

Driver.o: esc.o
	$(LD) -r -o Driver.o esc.o

esc.o: esc.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/mkdev.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/cred.h \
	$(INC)/sys/uio.h \
	$(INC)/sys/kmem.h \
	$(INC)/sys/bootinfo.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/ddi.h \
	../../sys/scsi.h \
	../../sys/sdi.h \
	$(FRC) \
	esc.h
	$(CC) $(CFLAGS) -c esc.c

FRC:
