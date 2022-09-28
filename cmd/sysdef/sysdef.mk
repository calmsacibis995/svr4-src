#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)sysdef:sysdef.mk	1.16.3.1"
#
#

ROOT =
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -I$(INCSYS) -s
LIBELF = -lelf
SYMLINK = :
INS = install
FRC =

all: sysdef

install: sysdef
	-rm -f $(ROOT)/etc/sysdef
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u bin -g bin sysdef
	-$(SYMLINK) /usr/sbin/sysdef $(ROOT)/etc/sysdef

sysdef: sysdef.c
	$(CC) $(CFLAGS) sysdef.c $(LIBELF) -o sysdef $(SHLIBS)

clobber: clean

clean:
	rm -f sysdef

FRC:

#
# Header dependencies
#

sysdef: sysdef.c \
	$(INC)/a.out.h \
	$(INC)/aouthdr.h \
	$(INC)/ctype.h \
	$(INC)/filehdr.h \
	$(INC)/ldfcn.h \
	$(INC)/linenum.h \
	$(INC)/nlist.h \
	$(INC)/reloc.h \
	$(INC)/scnhdr.h \
	$(INC)/stdio.h \
	$(INC)/storclass.h \
	$(INC)/syms.h \
	$(INCSYS)/sys/conf.h \
	$(INC)/dirent.h \
	$(INCSYS)/sys/ipc.h \
	$(INCSYS)/sys/mkdev.h \
	$(INCSYS)/sys/msg.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/sem.h \
	$(INCSYS)/sys/shm.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/sysi86.h \
	$(INCSYS)/sys/sysmacros.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/utsname.h \
	$(INCSYS)/sys/var.h \
	$(FRC)
