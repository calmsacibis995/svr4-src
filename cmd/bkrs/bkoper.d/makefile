#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkoper.d/makefile	1.16.2.1"

SRC1=bkoper.c list.c msgs.c utils.c
OBJ=$(SRC1:.c=.o)

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
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
FLAGS=
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=bkoper

all: $(PRODUCT)

bkoper: $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm $(LDLIBS)

clean:
	rm -f $(OBJ)

clobber:	clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS) $(SRC1)

strip:	$(PRODUCT)
	$(STRIP) $(PRODUCT)

install: $(PRODUCT)
	install -f $(USBIN) -u root -g sys -m 0100 $(PRODUCT)

bkoper.o: $(INC)/stdio.h
bkoper.o: $(INC)/sys/types.h
bkoper.o: $(INC)/ctype.h
bkoper.o: $(INC)/time.h
bkoper.o: $(INC)/bkrs.h
bkoper.o: $(GENERAL)/bktypes.h
bkoper.o: $(GENERAL)/backup.h
bkoper.o: $(GENERAL)/bkmsgs.h
bkoper.o: $(GENERAL)/bkerrors.h
bkoper.o: $(GENERAL)/bkstatus.h
bkoper.o: $(LOCAL)/bkoper.h
bkoper.o: $(LOCAL)/errors.h
list.o: $(INC)/sys/types.h
list.o: $(INC)/sys/stat.h
list.o: $(INC)/fcntl.h
list.o: $(INC)/time.h
list.o: $(INC)/stdio.h
list.o: $(INC)/table.h
list.o: $(INC)/devmgmt.h
list.o: $(GENERAL)/bktypes.h
list.o: $(GENERAL)/backup.h
list.o: $(GENERAL)/bkmsgs.h
list.o: $(LOCAL)/bkoper.h
list.o: $(GENERAL)/bkstatus.h
list.o: $(LOCAL)/errors.h
list.o: $(INC)/errno.h
utils.o: $(INC)/sys/types.h
utils.o: $(INC)/string.h
utils.o: $(INC)/pwd.h
