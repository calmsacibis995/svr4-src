#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/proc/proc.mk	1.1"

LIBRARY = libproc.a
HEADER1=../filecab/inc
INCLUDE = -I$(HEADER1)
AR=		ar
CFLAGS= 	-O



$(LIBRARY):	\
		$(LIBRARY)(suspend.o)

$(LIBRARY)(suspend.o):	suspend.c $(HEADER1)/wish.h

.c.a:
	$(CC) -c $(CFLAGS)  $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o


###### Standard makefile targets #######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
