#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkreg.d/makefile	1.15.2.1"

SRC=bkreg.c msgs.c
OBJ=$(SRC:.c=.o)

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
BIN=$(ROOT)/bin

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

PRODUCT=bkreg bkreg.tab .oam

all: $(PRODUCT)

bkreg: $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen $(LDLIBS)

bkreg.tab: table
	grep -v "^#ident" table >bkreg.tab

.oam: oam
	grep -v "^#ident" oam >.oam

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
	install -f $(USBIN) -u root -g sys -m 0100 bkreg
	install -f $(ETCBKUP) -u root -g sys -m 0600 bkreg.tab
	install -f $(ETCBKUP) -u root -g sys -m 0600 .oam

bkreg.o: $(INC)/string.h
bkreg.o: $(INC)/signal.h
bkreg.o: $(INC)/sys/types.h
bkreg.o: $(INC)/sys/stat.h
bkreg.o: $(INC)/stdio.h
bkreg.o: $(INC)/fcntl.h
bkreg.o: $(INC)/ctype.h
bkreg.o: $(INC)/table.h
bkreg.o: $(INC)/errno.h
bkreg.o: $(GENERAL)/backup.h
bkreg.o: $(GENERAL)/bkreg.h
bkreg.o: $(LOCAL)/errors.h
