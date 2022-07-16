#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1989 TOSHIBA Corporation
#	  All Rights Reserved

#	Copyright (c) 1989 SORD Computer Corporation
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	TOSHIBA Corporation and SORD Computer Corporation
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:io/sw01/Makefile	1.3.1.1"

ROOT =
MORECPP =
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL $(MORECPP)
FRC =

DRIVER =	Driver.o

all:		$(DRIVER)

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/sw01 ] || mkdir ../../pkg/sw01
	cp $(DRIVER) ../../pkg/sw01/
	cp sw01.h ../../pkg/sys/
	chmod 444 ../../pkg/sys/sw01.h
	chown bin ../../pkg/sys/sw01.h
	chgrp bin ../../pkg/sys/sw01.h

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f sw01.o

Driver.o: sw01.o
	$(LD) -r -o Driver.o sw01.o

sw01.o:   sw01.c \
	$(INC)/sys/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/param.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/user.h \
	$(INC)/sys/cmn_err.h \
	$(INC)/sys/buf.h \
	$(INC)/sys/immu.h \
	$(INC)/sys/systm.h \
	$(INC)/sys/open.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/debug.h \
	$(INC)/sys/conf.h \
	$(FRC) \
	../../sys/worm.h \
	../../sys/scsi.h \
	../../sys/sdi.h \
	../../sys/sdi_edt.h \
	sw01.h
	$(CC) $(CFLAGS) -c sw01.c
#	$(CC) $(CFLAGS) -c -DDEBUG sw01.c

FRC:
