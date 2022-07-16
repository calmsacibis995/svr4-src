#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibc:port/print/makefile	1.5.3.1"

#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.
#
# makefile for ucblibc/port/stdio
#
#

CC=cc
CFLAGS= -c -O
PROF=
NONPROF=
INC1=$(ROOT)/usr/ucbinclude
INC=$(ROOT)/usr/include
DEFLIST=
SDEFLIST=
INS=install



OBJECTS =  doprnt.o fprintf.o printf.o sprintf.o \
	vfprintf.o vprintf.o vsprintf.o fopen.o

SOURCES =  doprnt.c fprintf.c printf.c sprintf.c \
	vfprintf.c vprintf.c vsprintf.c fopen.c


ALL:		 $(OBJECTS)

fopen.o:	$(INC)/stdio.h \
		$(INC)/fcntl.h
		$(CC) $(CFLAGS) -I$(INC) fopen.c

fprintf.o:	$(INC)/stdio.h \
		$(INC)/stdarg.h
		$(CC) $(CFLAGS) -I$(INC) fprintf.c

printf.o:	$(INC)/stdio.h \
		$(INC)/stdarg.h
		$(CC) $(CFLAGS) -I$(INC) printf.c

sprintf.o:	$(INC1)/stdio.h \
		$(INC)/values.h \
		$(INC)/stdarg.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) sprintf.c

vfprintf.o:	$(INC)/stdio.h \
		$(INC)/stdarg.h
		$(CC) $(CFLAGS) -I$(INC) vfprintf.c

vprintf.o:	$(INC)/stdio.h \
		$(INC)/stdarg.h
		$(CC) $(CFLAGS) -I$(INC) vprintf.c

vsprintf.o:	$(INC1)/stdio.h \
		$(INC)/stdarg.h \
		$(INC)/values.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) vsprintf.c

doprnt.o:	$(INC)/stdio.h \
		$(INC)/stdlib.h \
		$(INC)/ctype.h \
		$(INC)/varargs.h \
		$(INC)/values.h \
		$(INC)/nan.h \
		$(INC)/memory.h \
		$(INC)/string.h \
		$(INC)/locale.h \
		print.h stdiom.h _locale.h
		$(CC) $(CFLAGS) -I$(INC) doprnt.c


clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS)
