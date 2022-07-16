#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:io/scsi/Makefile	1.3.1.1"

ROOT =
MORECPP = -DAT386 -D_SYSTEMENV=4
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL $(MORECPP)
FRC =

DRIVER =	Driver.o

all:		$(DRIVER)

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/scsi ] || mkdir ../../pkg/scsi
	cp $(DRIVER) ../../pkg/scsi/
	cp ad1542.h ../../pkg/sys/ 
	chmod 444 ../../pkg/sys/ad1542.h
	chown bin ../../pkg/sys/ad1542.h
	chgrp bin ../../pkg/sys/ad1542.h

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f ad1542.o

Driver.o: ad1542.o
	$(LD) -r -o Driver.o ad1542.o

ad1542.o: ad1542.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/bootinfo.h \
	$(INC)/sys/debug.h \
	../../sys/scsi.h \
	../../sys/sdi.h \
	$(FRC) \
	ad1542.h
	$(CC) $(CFLAGS) -c ad1542.c
#	$(CC) $(CFLAGS) -c -DDEBUG ad1542.c

FRC:
