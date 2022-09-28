#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devintf:mkdtab/mkdtab.mk	1.1.3.1"

INSDIR = $(ROOT)/usr/sadm/sysadm/bin
INC = $(ROOT)/usr/include
LDFLAGS = -s  -ladm $(SHLIBS)
CFLAGS = -O -I$(INC)
INS = install
ARCH=AT386
BUS=AT386

all: mkdtab

mkdtab: mkdtab.c\
	$(INC)/stdio.h\
	$(INC)/stdlib.h\
	$(INC)/string.h\
	$(INC)/fcntl.h\
	$(INC)/unistd.h\
	$(INC)/devmgmt.h\
	$(INC)/sys/mkdev.h\
	$(INC)/sys/cram.h\
	$(INC)/sys/param.h\
	$(INC)/sys/stat.h\
	$(INC)/sys/vtoc.h\
	$(INC)/sys/vfstab.h\
	$(INC)/sys/fs/s5filsys.h\
	mkdtab.h
	$(CC) $(CFLAGS) -D$(ARCH) -D$(BUS) mkdtab.c -o mkdtab $(LDFLAGS)

install: all
	[ -d $(INSDIR) ] || mkdir -p $(INSDIR)
	$(INS) -f $(INSDIR) -u bin -g bin -m 555 mkdtab

clean:
	rm -f *.o

clobber: clean
	rm -f mkdtab
