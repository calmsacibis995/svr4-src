#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:cmd/Makefile	1.3.1.1"

ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -I.. -I$(INC) -Di386
LDFLAGS = -s
FRC =
FORMAT_FILES = sd00.0 sd01.1

SCSICMDS = scsiformat DISK

FORMAT = \
        tc.index \
        format.d/sd00.0 \
        format.d/sd01.1

all:	$(SCSICMDS) $(FORMAT)

install:	all
	[ -d ../pkg ] || mkdir ../pkg
	[ -d ../pkg/cmd ] || mkdir ../pkg/cmd
	[ -d ../pkg/cmd/format.d ] || mkdir ../pkg/cmd/format.d
	-ls $(SCSICMDS) | cpio -pduv ../pkg/cmd/
	grep -v "^#ident" tc.index > ../pkg/cmd/tc.index ;\
	for i in $(FORMAT_FILES); \
	do \
		grep -v "^#ident" format.d/$$i > ../pkg/cmd/format.d/$$i ;\
	done
clean:
	rm -f *.o $(SCSICMDS)

clobber: clean
	rm -f $(SCSICMDS)

scsiformat: format.o scl.o script.o scsicomm.o
	$(CC) $(LDFLAGS) -o scsiformat format.o scl.o script.o scsicomm.o

DISK: DISK.o scl.o script.o scsicomm.o
	$(CC) $(LDFLAGS) -o DISK DISK.o scl.o script.o scsicomm.o

format.o: format.c \
	$(INC)/stdio.h \
	$(INC)/utmp.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/vtoc.h \
	tokens.h \
	scl.h 
	$(CC) $(CFLAGS) -c format.c

scl.o:	scl.c \
	$(INC)/stdio.h \
	$(INC)/errno.h \
	$(INC)/sys/types.h \
	$(INC)/sys/errno.h \
	$(INC)/fcntl.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/mkdev.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/vtoc.h \
	scsicomm.h \
	$(INC)/string.h \
	$(INC)/signal.h \
	$(INC)/sys/signal.h \
	../sys/scsi.h \
	../sys/sdi.h \
	../sys/sdi_edt.h \
	tokens.h \
	scl.h
	$(CC) $(CFLAGS) -c scl.c

script.o: script.c \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/errno.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/stat.h \
	../sys/scsi.h \
	tokens.h
	$(CC) $(CFLAGS) -c script.c

DISK.o: DISK.c \
	$(INC)/stdio.h \
	$(INC)/errno.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/mnttab.h \
	$(INC)/utmp.h \
	$(INC)/sys/types.h \
	$(INC)/signal.h \
	$(INC)/sys/signal.h \
	$(INC)/fcntl.h \
	$(INC)/sys/fcntl.h \
	$(INC)/sys/vtoc.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/param.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/filsys.h \
	$(INC)/sys//fs/s5macros.h \
	$(INC)/sys/fdisk.h \
	../sys/scsi.h \
	../sys/sd01_ioctl.h \
	../sys/mirror.h \
	scl.h \
	tokens.h \
	scsicomm.h 
	$(CC) $(CFLAGS) -c DISK.c

scsicomm.o: scsicomm.c \
	$(INC)/sys/types.h \
	$(INC)/sys/mkdev.h \
	$(INC)/sys/stat.h \
	../sys/sdi_edt.h \
	../sys/mirror.h \
	$(INC)/fcntl.h \
	$(INC)/sys/fcntl.h \
	$(INC)/errno.h \
	$(INC)/sys/errno.h \
	$(INC)/sys/vtoc.h \
	../sys/sd01_ioctl.h \
	$(INC)/string.h \
	$(INC)/stdio.h \
	scsicomm.h 
	$(CC) $(CFLAGS) -c scsicomm.c

FRC:
