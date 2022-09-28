#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsstatus.d/makefile	1.10.2.1"

SRC=rsstatus.c admin_auth.c msgs.c
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
FLAGS=-DTRACE
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=rsstatus ursstatus

all: $(PRODUCT)

rsstatus:  $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen

ursstatus: rsstatus
	ln rsstatus ursstatus

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
	install -f $(SBIN) -u root -g sys -m 0100 rsstatus
	install -f $(SBIN) -u root -g sys -m 4111 ursstatus

rsstatus.o: $(INC)/stdio.h
rsstatus.o: $(INC)/fcntl.h
rsstatus.o: $(INC)/string.h
rsstatus.o: $(INC)/errno.h
rsstatus.o: $(INC)/sys/types.h
rsstatus.o: $(INC)/sys/stat.h
rsstatus.o: $(INC)/time.h
rsstatus.o: $(INC)/pwd.h
rsstatus.o: $(INC)/table.h
rsstatus.o: $(GENERAL)/rsstatus.h
rsstatus.o: $(GENERAL)/bktypes.h
rsstatus.o: $(LOCAL)/errors.h
