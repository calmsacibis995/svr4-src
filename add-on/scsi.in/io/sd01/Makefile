#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:io/sd01/Makefile	1.3"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL -D_LTYPES
FRC =

DRIVER =	Driver.o

all:		$(DRIVER)

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/sd01 ] || mkdir ../../pkg/sd01
	cp $(DRIVER) ../../pkg/sd01/
	cp ../../sys/sd01_ioctl.h ../../pkg/sys/
	cp sd01.h ../../pkg/sys/

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f $(DRIVER)
	rm -f sd01.o

Driver.o: sd01.o
	$(LD) -r -o Driver.o sd01.o

sd01.o:   sd01.c \
	$(INC)/sys/types.h \
	$(INC)/sys/fdisk.h \
	$(INC)/sys/param.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/bootinfo.h \
	$(INC)/sys/elog.h \
	$(INC)/sys/open.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/ddi.h \
	$(INC)/sys/cred.h \
	$(FRC) \
	../../sys/sdi.h \
	../../sys/scsi.h \
	../../sys/sd01_ioctl.h \
	sd01.h
	$(CC) $(CFLAGS) -c sd01.c
#	$(CC) $(CFLAGS) -c -DDEBUG sd01.c

FRC:
