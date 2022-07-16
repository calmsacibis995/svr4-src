#ident	"@(#)Makefile	1.2	92/06/28	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)qt:io/Makefile	1.3"
#

all:	Driver.o 

install package:	all 
	-ls Driver.o qt.h | cpio -pduv ../pkg

clean: $(FRC)
	rm -f driver.o

clobber: clean
	rm -f *.o

Driver.o:	driver.o 
	$(LD) -r -o Driver.o driver.o

driver.o:	driver.c qt.h globals.h debug.h $(FRC)
	$(CC) -c -O driver.c
#	$(CC) -c -DDEBUG driver.c

FRC:
