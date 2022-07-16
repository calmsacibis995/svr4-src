#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pc586:io/Makefile	1.3"

INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -DINKERNEL -DSYSV $(MORECPP) -I..
DRIVERS = pc586
FRC =

COMFILES =\
	Drvr.pc586.o 

all:	$(COMFILES)

install: all
	-for driver in $(DRIVERS) ; \
	do \
		[ -d ../pkg ] || mkdir ../pkg ; \
		cp Drvr.$$driver.o ../pkg/Driver.o ; \
	done

clean:
	-for driver in $(DRIVERS) ; \
	do \
		rm -f ../pkg/Driver.o ; \
	done
	rm -f *.o $(COMFILES) 

clobber:	clean

FRC:

Drvr.pc586.o:	pc586.o pc586bcopy.o $(FRC)
	$(LD) -r -o $@ pc586.o pc586bcopy.o
