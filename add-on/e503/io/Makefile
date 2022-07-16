#ident	"@(#)Makefile	1.1	92/09/30	JPB"

#      @(#)e3B.mk	3.3 Lachman System V STREAMS TCP  source
#
#	System V STREAMS TCP - Release 3.0
#
#	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
#
#	All Rights Reserved.
#
#	The copyright above and this notice must be preserved in all
#	copies of this source code.  The copyright above does not
#	evidence any actual or intended publication of this source
#	code.
#
#	This is unpublished proprietary trade secret source code of
#	Lachman Associates.  This source code may not be copied,
#	disclosed, distributed, demonstrated or licensed except as
#	expressly authorized by Lachman Associates.
#
#	System V STREAMS TCP was jointly developed by Lachman
#	Associates and Convergent Technologies.
#
#
#	System V STREAMS TCP - Release 3.0
#
#	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
#	All Rights Reserved.
#
#	The copyright above and this notice must be preserved in all
#	copies of this source code.  The copyright above does not
#	evidence any actual or intended publication of this source
#	code.
#
#	This is unpublished proprietary trade secret source code of
#	Lachman Associates.  This source code may not be copied,
#	disclosed, distributed, demonstrated or licensed except as
#	expressly authorized by Lachman Associates.
#
#	System V STREAMS TCP was jointly developed by Lachman
#	Associates and Convergent Technologies.
#

INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -D_KERNEL -DSYSV -DSVR40 -DV4_IFSTAT $(MORECPP) -I..
DRIVERS = 3c503
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

Driver.o:	e503li.o e503mac.o e503io.o $(FRC)
	$(LD) -r -o $@ e503li.o e503mac.o e503io.o

e503li.o:	e503li.c \
	$(INC)/sys/conf.h \
	$(INC)/sys/dir.h \
	$(INC)/sys/lihdr.h \
	$(INC)/sys/log.h \
	$(INC)/sys/page.h \
	$(INC)/sys/param.h \
	$(INC)/sys/seg.h \
	$(INC)/sys/signal.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/strlog.h \
	$(INC)/sys/stropts.h \
	$(INC)/sys/sysmacros.h \
	$(INC)/sys/types.h \
	$(INC)/sys/user.h \
	../sys/e503.h \
	$(INC)/sys/errno.h \
	$(INC)/net/if.h \
	$(FRC)
	$(CC) -c $(CFLAGS) e503li.c

e503mac.o:	e503mac.c \
	$(INC)/sys/log.h \
	$(INC)/sys/page.h \
	$(INC)/sys/param.h \
	$(INC)/sys/stream.h \
	$(INC)/sys/strlog.h \
	$(INC)/sys/sysmacros.h \
	../sys/e503.h \
	$(INC)/net/if.h \
	$(INC)/sys/socket.h \
	$(INC)/sys/types.h \
	$(FRC)
	$(CC) -c $(CFLAGS) e503mac.c

e503io.o:	e503io.s \
	$(FRC)
	$(CC) -c $(CFLAGS) e503io.s

lint:
	lint $(CFLAGS) e503li.c e503mac.c > lint.out 2>&1
