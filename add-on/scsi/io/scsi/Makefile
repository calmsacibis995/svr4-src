#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:io/scsi/Makefile	1.3"

#		SCSI Host Adaptor Driver

ROOT =
MORECPP = -DAT386
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL -D_LTYPES $(MORECPP)
FRC =


DRIVER =	Driver.o Driver2.o Driver3.o

all:		$(DRIVER)

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/scsi ] || mkdir ../../pkg/scsi
	cp $(DRIVER) ../../pkg/scsi/
	cp ../../sys/scsi.h ../../pkg/sys/
	cp ../../sys/sdi.h ../../pkg/sys/ 
	cp ../../sys/sdi_edt.h ../../pkg/sys/ 
	cp had.h ../../pkg/sys/ 

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f $(DRIVER)
	rm -f scsi.o
	rm -f Driver2.o
	rm -f Driver3.o

Driver2.o: Driver2.c
	$(CC) -c Driver2.c

Driver3.o: Driver3.c
	$(CC) -c Driver3.c

Driver.o: scsi.o
	$(LD) -r -o Driver.o scsi.o

scsi.o:   scsi.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/ddi.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/kmem.h \
	$(INC)/sys/bootinfo.h \
	../../sys/scsi.h \
	../../sys/sdi.h \
	had.h \
	$(FRC) \
	wd7000.h
	$(CC) $(CFLAGS) -c scsi.c
#	$(CC) -c $(CFLAGS) -DDEBUG scsi.c

FRC:
