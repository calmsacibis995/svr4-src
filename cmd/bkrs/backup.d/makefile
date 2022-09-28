#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:backup.d/makefile	1.1.4.1"

SRC=backup.c msgs.c print.c queue.c bkregvalid.c
OBJ=$(SRC:.c=.o)

INC=$(ROOT)/usr/include
ETC=$(ROOT)/etc
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
CC=$(PFX)cc
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a
LOCAL=hdrs
GENERAL=../hdrs
FLAGS=
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=backup

all: $(PRODUCT)

backup: $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen $(LDLIBS)

clean:
	rm -f $(OBJ)

clobber:	clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS)  $(SRC)

strip:	$(PRODUCT)
	$(STRIP) $(PRODUCT)

install: $(PRODUCT)
	install -f $(USBIN) -u root -g sys -m 0100 $(PRODUCT)

backup.o: $(INC)/sys/types.h
backup.o: $(INC)/time.h
backup.o: $(INC)/stdio.h
backup.o: $(INC)/setjmp.h
backup.o: $(INC)/pwd.h
backup.o: $(GENERAL)/backup.h
backup.o: $(GENERAL)/bkmsgs.h
backup.o: $(GENERAL)/bkreg.h
backup.o: $(INC)/bkrs.h
backup.o: $(LOCAL)/errors.h
backup.o: $(INC)/signal.h
backup.o: $(INC)/errno.h
print.o: $(INC)/time.h
print.o: $(INC)/stdio.h
print.o: $(GENERAL)/backup.h
print.o: $(GENERAL)/bkmsgs.h
queue.o: $(INC)/sys/types.h
queue.o: $(INC)/time.h
queue.o: $(GENERAL)/backup.h
queue.o: $(GENERAL)/bkmsgs.h
bkregvalid.o: $(INC)/stdio.h
bkregvalid.o: $(INC)/fcntl.h
bkregvalid.o: $(INC)/table.h
bkregvalid.o: $(INC)/string.h
bkregvalid.o: $(GENERAL)/backup.h
bkregvalid.o: $(INC)/sys/types.h
bkregvalid.o: $(GENERAL)/bkmsgs.h
bkregvalid.o: $(LOCAL)/errors.h
bkregvalid.o: $(GENERAL)/bkerrors.h
bkregvalid.o: $(GENERAL)/bkreg.h
