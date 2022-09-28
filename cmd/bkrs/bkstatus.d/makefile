#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkstatus.d/makefile	1.8.2.1"

SRC=bkstatus.c msgs.c
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

PRODUCT=bkstatus

all: $(PRODUCT)

bkstatus:  $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen $(LDLIBS)

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
	install -f $(USBIN) -u root -g sys -m 0100 bkstatus

bkstatus.o: $(INC)/stdio.h
bkstatus.o: $(INC)/fcntl.h
bkstatus.o: $(INC)/string.h
bkstatus.o: $(INC)/errno.h
bkstatus.o: $(INC)/sys/types.h
bkstatus.o: $(INC)/sys/stat.h
bkstatus.o: $(INC)/time.h
bkstatus.o: $(INC)/pwd.h
bkstatus.o: $(INC)/ctype.h
bkstatus.o: $(INC)/table.h
bkstatus.o: $(GENERAL)/bkstatus.h
bkstatus.o: $(GENERAL)/backup.h
bkstatus.o: $(GENERAL)/bktypes.h
bkstatus.o: $(LOCAL)/errors.h
