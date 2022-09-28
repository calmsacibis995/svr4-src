#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rsoper.d/makefile	1.16.2.1"

SRC1=cancel.c entry.c list.c mail.c msgs.c \
	rsoper.c auto.c utils.c
OBJ=$(SRC1:.c=.o)

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
RSLIB=../rslib.d/rslib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a
LOCAL=hdrs
GENERAL=../hdrs
FLAGS=
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=rsoper

all: $(PRODUCT)

rsoper: $(OBJ) $(RSLIB) $(BKLIB) $(LIBBR) $(IOLIB) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) $(RSLIB) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen -ladm

clean:
	rm -f $(OBJ)

clobber:	clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS)  $(SRC1)

strip:	$(PRODUCT)
	$(STRIP) $(PRODUCT)

install: $(PRODUCT)
	install -f $(SBIN) -u root -g sys -m 0100 $(PRODUCT)
	install -f $(USBIN) -u root -g sys -m 0100 $(PRODUCT)

cancel.o: $(INC)/sys/types.h
cancel.o: $(INC)/fcntl.h
cancel.o: $(INC)/stdio.h
cancel.o: $(INC)/errno.h
cancel.o: $(INC)/table.h
cancel.o: $(GENERAL)/rsstatus.h
cancel.o: $(LOCAL)/errors.h
entry.o: $(INC)/sys/types.h
entry.o: $(INC)/stdio.h
entry.o: $(INC)/table.h
entry.o: $(GENERAL)/bktypes.h
entry.o: $(GENERAL)/restore.h
entry.o: $(GENERAL)/rsstatus.h
entry.o: $(LOCAL)/rsoper.h
entry.o: $(LOCAL)/errors.h
list.o: $(INC)/sys/types.h
list.o: $(INC)/fcntl.h
list.o: $(INC)/stdio.h
list.o: $(INC)/string.h
list.o: $(INC)/pwd.h
list.o: $(INC)/errno.h
list.o: $(INC)/table.h
list.o: $(GENERAL)/bktypes.h
list.o: $(GENERAL)/restore.h
list.o: $(GENERAL)/rsstatus.h
list.o: $(LOCAL)/rsoper.h
list.o: $(LOCAL)/errors.h
mail.o: $(INC)/sys/types.h
mail.o: $(INC)/stdio.h
mail.o: $(INC)/errno.h
mail.o: $(LOCAL)/errors.h
rsoper.o: $(INC)/sys/types.h
rsoper.o: $(INC)/signal.h
rsoper.o: $(INC)/stdio.h
rsoper.o: $(INC)/string.h
rsoper.o: $(INC)/bkrs.h
rsoper.o: $(GENERAL)/backup.h
rsoper.o: $(GENERAL)/bktypes.h
rsoper.o: $(GENERAL)/bkerrors.h
rsoper.o: $(GENERAL)/restore.h
rsoper.o: $(LOCAL)/rsoper.h
rsoper.o: $(LOCAL)/errors.h
auto.o: $(INC)/fcntl.h
auto.o: $(INC)/time.h
auto.o: $(INC)/errno.h
auto.o: $(INC)/table.h
auto.o: $(GENERAL)/restore.h
auto.o: $(GENERAL)/rsstatus.h
auto.o: $(GENERAL)/bkhist.h
auto.o: $(GENERAL)/brtoc.h
utils.o: $(INC)/sys/types.h
utils.o: $(INC)/time.h
utils.o: $(INC)/string.h
utils.o: $(INC)/pwd.h
utils.o: $(INC)/bkrs.h
utils.o: $(GENERAL)/brarc.h
utils.o: $(GENERAL)/bktypes.h
queue.o: $(INC)/sys/types.h
queue.o: $(INC)/time.h
queue.o: $(GENERAL)/backup.h
queue.o: $(GENERAL)/bkmsgs.h
