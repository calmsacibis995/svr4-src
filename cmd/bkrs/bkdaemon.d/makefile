#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkdaemon.d/makefile	1.1.4.1"

SRC1= bkdaemon.c control.c entry.c file.c incoming.c mail.c message.c method.c \
	owner.c proc.c queue.c space.c status.c table.c utils.c operator.c history.c \
	labels.c eventq.c events.c
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
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a
LOCAL=hdrs
GENERAL=../hdrs
FLAGS=
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=bkdaemon

all: $(PRODUCT)

bkdaemon: $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJ) -lTL $(BKLIB) $(LIBBR) $(IOLIB) -lgen -ladm $(LDLIBS)

clean:
	rm -f $(OBJ)

clobber:	clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -h $(LFLAGS) $(CFLAGS) $(SRC1)

strip:	$(PRODUCT)
	$(STRIP) $(PRODUCT)

install: $(PRODUCT)
	install -f $(BKBIN) -u root -g sys -m 0100 $(PRODUCT)

bkdaemon.o: $(INC)/stdio.h
bkdaemon.o: $(INC)/sys/types.h
bkdaemon.o: $(INC)/signal.h
bkdaemon.o: $(GENERAL)/backup.h
bkdaemon.o: $(GENERAL)/bkmsgs.h
bkdaemon.o: $(INC)/bkrs.h
bkdaemon.o: $(LOCAL)/bkdaemon.h
bkdaemon.o: $(INC)/errno.h
control.o: $(INC)/sys/types.h
control.o: $(GENERAL)/backup.h
control.o: $(GENERAL)/bkerrors.h
control.o: $(GENERAL)/bkmsgs.h
control.o: $(LOCAL)/bkdaemon.h
entry.o: $(INC)/sys/types.h
entry.o: $(GENERAL)/backup.h
entry.o: $(GENERAL)/bkmsgs.h
entry.o: $(LOCAL)/bkdaemon.h
entry.o: $(GENERAL)/bkreg.h
entry.o: $(INC)/table.h
file.o: $(INC)/sys/types.h
file.o: $(INC)/sys/stat.h
incoming.o: $(INC)/sys/types.h
incoming.o: $(GENERAL)/backup.h
incoming.o: $(GENERAL)/bkmsgs.h
incoming.o: $(LOCAL)/bkdaemon.h
incoming.o: $(GENERAL)/bkstatus.h
incoming.o: $(GENERAL)/bkerrors.h
mail.o: $(INC)/sys/types.h
mail.o: $(INC)/pwd.h
mail.o: $(INC)/stdio.h
mail.o: $(GENERAL)/backup.h
mail.o: $(GENERAL)/bkmsgs.h
mail.o: $(LOCAL)/bkdaemon.h
mail.o: $(GENERAL)/bkerrors.h
mail.o: $(INC)/errno.h
message.o: $(INC)/sys/types.h
message.o: $(GENERAL)/backup.h
message.o: $(GENERAL)/bkmsgs.h
message.o: $(LOCAL)/bkdaemon.h
message.o: $(INC)/errno.h
method.o: $(INC)/sys/types.h
method.o: $(INC)/fcntl.h
method.o: $(INC)/ctype.h
method.o: $(INC)/table.h
method.o: $(INC)/string.h
method.o: $(INC)/bkrs.h
method.o: $(GENERAL)/backup.h
method.o: $(GENERAL)/bkmsgs.h
method.o: $(LOCAL)/bkdaemon.h
method.o: $(GENERAL)/bkerrors.h
method.o: $(GENERAL)/bkstatus.h
method.o: $(INC)/errno.h
method.o: $(GENERAL)/bktypes.h
owner.o: $(INC)/sys/types.h
owner.o: $(GENERAL)/bktypes.h
owner.o: $(GENERAL)/backup.h
owner.o: $(GENERAL)/bkmsgs.h
owner.o: $(LOCAL)/bkdaemon.h
owner.o: $(GENERAL)/bkerrors.h
proc.o: $(INC)/sys/types.h
proc.o: $(GENERAL)/backup.h
proc.o: $(GENERAL)/bkmsgs.h
proc.o: $(LOCAL)/bkdaemon.h
proc.o: $(GENERAL)/bkerrors.h
proc.o: $(INC)/errno.h
queue.o: $(INC)/sys/types.h
queue.o: $(GENERAL)/backup.h
queue.o: $(GENERAL)/bkmsgs.h
queue.o: $(LOCAL)/bkdaemon.h
queue.o: $(GENERAL)/bkerrors.h
queue.o: $(GENERAL)/bkstatus.h
space.o: $(INC)/sys/types.h
space.o: $(GENERAL)/backup.h
space.o: $(GENERAL)/bkmsgs.h
space.o: $(LOCAL)/bkdaemon.h
status.o: $(INC)/fcntl.h
status.o: $(INC)/sys/types.h
status.o: $(INC)/signal.h
status.o: $(INC)/table.h
status.o: $(GENERAL)/backup.h
status.o: $(GENERAL)/bkmsgs.h
status.o: $(LOCAL)/bkdaemon.h
status.o: $(GENERAL)/bkstatus.h
status.o: $(INC)/errno.h
table.o: $(INC)/sys/types.h
table.o: $(INC)/time.h
table.o: $(INC)/table.h
table.o: $(GENERAL)/backup.h
table.o: $(GENERAL)/bkreg.h
utils.o: $(INC)/sys/types.h
utils.o: $(INC)/string.h
utils.o: $(GENERAL)/backup.h
utils.o: $(GENERAL)/bkmsgs.h
utils.o: $(LOCAL)/bkdaemon.h
operator.o: $(INC)/sys/types.h
operator.o: $(GENERAL)/backup.h
operator.o: $(LOCAL)/bkdaemon.h
operator.o: $(GENERAL)/bkerrors.h
operator.o: $(GENERAL)/bkmsgs.h
history.o: $(INC)/sys/types.h
history.o: $(INC)/string.h
history.o: $(INC)/fcntl.h
history.o: $(INC)/table.h
history.o: $(GENERAL)/backup.h
history.o: $(LOCAL)/bkdaemon.h
history.o: $(GENERAL)/bkmsgs.h
history.o: $(GENERAL)/bkhist.h
history.o: $(GENERAL)/bktypes.h
history.o: $(INC)/errno.h
labels.o: $(INC)/sys/types.h
labels.o: $(GENERAL)/bktypes.h
labels.o: $(GENERAL)/backup.h
labels.o: $(LOCAL)/bkdaemon.h
labels.o: $(GENERAL)/bkerrors.h
eventq.o: $(INC)/sys/types.h
eventq.o: $(INC)/signal.h
eventq.o: $(GENERAL)/backup.h
eventq.o: $(LOCAL)/bkdaemon.h
events.o: $(INC)/sys/types.h
events.o: $(GENERAL)/backup.h
events.o: $(GENERAL)/bkmsgs.h
events.o: $(LOCAL)/bkdaemon.h
events.o: $(GENERAL)/bkerrors.h
