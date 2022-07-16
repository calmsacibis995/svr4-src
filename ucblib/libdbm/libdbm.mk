#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibdbm:libdbm.mk	1.3.3.1"

MAKEFILE = libdbm.mk

USRLIB=$(ROOT)/usr/ucblib

INC1=$(ROOT)/usr/ucbinclude

INC=$(ROOT)/usr/include

INCSYS=$(ROOT)/usr/include

INS=install

LIBRARY = libdbm.a

OBJECTS =  dbm.o

SOURCES =  dbm.c

ALL:		$(LIBRARY)

$(LIBRARY):	dbm.o
		$(AR) rv $(LIBRARY) dbm.o


dbm.o:		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/stat.h \
		 $(INCSYS)/sys/types.h \
		 $(INC1)/dbm.h 
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c dbm.c

GLOBALINCS = $(INCSYS)/sys/select.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/types.h \
	$(INC1)/dbm.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(LIBRARY)

install: ALL
	$(INS) -f $(USRLIB) -m 644 $(LIBRARY)
