#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:libadmIO.d/makefile	1.6.4.1"

SRC=libadmIO.c 
OBJ=$(SRC:.c=.o)

INS=install
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
GINCSYS=$(ROOT)/usr/include/sys
LOCAL=hdrs
GENERAL=../hdrs
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
LIBDIR=$(ROOT)/usr/lib/libadmIO
USRLIB=$(ROOT)/usr/lib
CC=$(PFX)cc
FLAGS=
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)
LIBRARY = libadmIO.a
LDFLAGS=$(LDLIBPATH)
MAKE=make
STRIP=$(PFX)strip
LORDER=$(PFX)lorder
LINT=$(PFX)lint

PRODUCTS=libadmIO.a

all: $(PRODUCTS)

libadmIO.a: libadmIO.o
	$(AR) cr $(@) libadmIO.o

touch: 
	touch $(SRC)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(PRODUCTS)

lintit:
	$(LINT) $(CFLAGS) $(LFLAGS) $(SRC)

strip:	$(PRODUCTS)
	$(STRIP) $(PRODUCTS)

install: $(PRODUCTS)
	$(INS) -f $(USRLIB) -m 644 $(LIBRARY)
	$(INS) -f $(GINCSYS) -m 444 $(GENERAL)/libadmIO.h

$(LIBRARY)(libadmIO.o):	$(INC)/sys/types.h \
			$(INC)/stdio.h \
			$(INC)/fcntl.h \
			$(INC)/sys/errno.h \
			$(INC)/signal.h \
			$(INC)/string.h \
			$(GENERAL)/libadmIO.h  \
			$(INC)/sys/stat.h \
			$(INC)/sys/statvfs.h \
			$(INC)/sys/mkdev.h \
			$(INC)/sys/stropts.h \
			$(INC)/malloc.h \
			$(INC)/memory.h 
