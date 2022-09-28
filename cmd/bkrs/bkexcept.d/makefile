#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkexcept.d/makefile	1.10.3.1"

SRC=bkexcept.c msgs.c
OBJ=$(SRC:.c=.o)

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
BIN=$(ROOT)/bin
UBIN=$(ROOT)/usr/bin

BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
CC=$(PFX)cc
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint

LOCAL=hdrs
GENERAL=../hdrs
FLAGS=
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=bkexcept bkexcept.tab

all: $(PRODUCT)

bkexcept:  $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen $(LDLIBS)

bkexcept.tab: table
	grep -v "^#ident" table >bkexcept.tab

touch: 
	touch $(SRC)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) $(LFLAGS) $(CFLAGS) $(SRC)

strip:	$(PRODUCT)
	$(STRIP) $(PRODUCT)

install: $(PRODUCT)
	install -f $(USBIN) -u root -g sys -m 0100 bkexcept
	install -f $(BKBIN) -u root -g sys -m 0400 exconv.sed
	install -f $(ETCBKUP) -u root -g sys -m 0600 bkexcept.tab

bkexcept.o: $(INC)/stdio.h
bkexcept.o: $(INC)/signal.h
bkexcept.o: $(INC)/fcntl.h
bkexcept.o: $(INC)/string.h
bkexcept.o: $(INC)/errno.h
bkexcept.o: $(INC)/sys/types.h
bkexcept.o: $(INC)/sys/stat.h
bkexcept.o: $(INC)/table.h
bkexcept.o: $(GENERAL)/bkexcept.h
bkexcept.o: $(GENERAL)/bktypes.h
bkexcept.o: $(LOCAL)/errors.h
