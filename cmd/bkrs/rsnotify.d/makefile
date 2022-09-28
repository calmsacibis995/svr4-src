#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsnotify.d/makefile	1.7.2.1"

SRC=rsnotify.c msgs.c
OBJ=$(SRC:.c=.o)

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
BIN=$(ROOT)/bin
UBIN=$(ROOT)/usr/bin
ETC=$(ROOT)/etc

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
FLAGS=-DTRACE
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=rsnotify

all: $(PRODUCT)

rsnotify:  $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) $(ULIB)/libTL.a
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
	install -f $(USBIN) -u root -g sys -m 0100 rsnotify

rsnotify.o: $(INC)/stdio.h
rsnotify.o: $(INC)/fcntl.h
rsnotify.o: $(INC)/string.h
rsnotify.o: $(INC)/sys/types.h
rsnotify.o: $(INC)/sys/stat.h
rsnotify.o: $(INC)/time.h
rsnotify.o: $(INC)/pwd.h
rsnotify.o: $(INC)/table.h
rsnotify.o: $(GENERAL)/rsnotify.h
rsnotify.o: $(LOCAL)/errors.h
