#ident	"@(#)Makefile	1.2	92/03/01	JPB"

#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  Copyrighted as an unpublished work.
#  (c) Copyright 1990 INTERACTIVE Systems Corporation
#  All rights reserved.
# 
#  RESTRICTED RIGHTS
# 
#  These programs are supplied under a license.  They may be used,
#  disclosed, and/or copied only as permitted under such license
#  agreement.  Any copy must contain the above copyright notice and
#  this restricted rights notice.  Use, copying, and/or disclosure
#  of the programs is strictly prohibited unless otherwise provided
#  in the license agreement.

#ident	"@(#)wd:io/Makefile	1.1.1.1"

INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -D_KERNEL -DSYSV -DSVR40 -DV4_IFSTAT $(MORECPP) -I..
DRIVERS = wd
FRC =

all:	Driver.o

install: all
	[ -d ../pkg ] || mkdir ../pkg
	cp Driver.o ../pkg

clean:
	rm -f ../pkg/Driver.o
	rm -f *.o

clobber:	clean

FRC:

Driver.o:	wd.o wdbcopy.o $(FRC)
	$(LD) -r -o $@ wd.o wdbcopy.o

wd.o:	wd.c			\
	$(INC)/sys/dir.h	\
	$(INC)/sys/errno.h	\
	$(INC)/sys/fs/s5dir.h	\
	$(INC)/sys/fs/s5param.h	\
	$(INC)/sys/immu.h	\
	$(INC)/sys/lihdr.h	\
	$(INC)/sys/param.h	\
	$(INC)/sys/signal.h	\
	$(INC)/sys/stream.h	\
	$(INC)/sys/stropts.h	\
	$(INC)/sys/sysmacros.h	\
	$(INC)/sys/types.h	\
	$(INC)/sys/user.h	\
	$(INC)/sys/kmem.h	\
	../sys/wd.h		\
	../sys/wdhdw.h		\
	$(FRC)
	$(CC) -c $(CFLAGS) wd.c

lint:
	lint $(CFLAGS) wd.c > lint.out 2>&1
