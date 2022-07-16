#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:io/st01/Makefile	1.3.1.1"

ROOT =
MORECPP = -DAT386 -D_SYSTEMENV=4
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL $(MORECPP)
FRC =

DRIVER =	Driver.o

all:		$(DRIVER)
	cp st01.h ../../sys/
	chmod 444 ../../sys/st01.h
	chown bin ../../sys/st01.h
	chgrp bin ../../sys/st01.h

install:	all
	[ -d ../../pkg ] || mkdir ../../pkg
	[ -d ../../pkg/sys ] || mkdir ../../pkg/sys
	[ -d ../../pkg/st01 ] || mkdir ../../pkg/st01
	cp $(DRIVER) ../../pkg/st01/
	cp st01.h ../../pkg/sys/
	chmod 444 ../../pkg/sys/st01.h
	chown bin ../../pkg/sys/st01.h
	chgrp bin ../../pkg/sys/st01.h

clean:
	rm -f $(DRIVER)
clobber: clean
	rm -f st01.o

Driver.o: st01.o
	$(LD) -r -o Driver.o st01.o

st01.o:   st01.c \
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
	$(INC)/sys/file.h \
	$(INC)/sys/open.h \
	$(INC)/sys/ioctl.h \
	$(INC)/sys/debug.h \
	$(FRC) \
	$(INC)/sys/tape.h \
	../../sys/scsi.h \
	../../sys/sdi.h \
	../../sys/sdi_edt.h \
	st01.h
	$(CC) $(CFLAGS) -c st01.c
#	$(CC) $(CFLAGS) -c -DDEBUG st01.c

FRC:
