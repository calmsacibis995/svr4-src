#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bkhistory.d/makefile	1.8.3.1"

SRC=bkhistory.c msgs.c
OBJ=$(SRC:.c=.o)

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a

OAMBASE=$(ROOT)/usr/oam
INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
ULIB=$(ROOT)/usr/lib
BIN=$(ROOT)/bin
UBIN=$(ROOT)/usr/bin
ETC=$(ROOT)/etc
SBIN=$(ROOT)/sbin

CC=$(PFX)cc
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint

LOCAL=hdrs
GENERAL=../hdrs
FLAGS=-DTRACE
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=bkhistory

all: $(PRODUCT)

bkhistory:  $(OBJ) $(BKLIB) $(IOLIB) $(LIBBR) $(ULIB)/libTL.a
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
	install -f $(SBIN) -u root -g sys -m 0100 $(PRODUCT)

bkhistory.o: $(INC)/stdio.h
bkhistory.o: $(INC)/fcntl.h
bkhistory.o: $(INC)/string.h
bkhistory.o: $(INC)/sys/types.h
bkhistory.o: $(INC)/sys/stat.h
bkhistory.o: $(INC)/time.h
bkhistory.o: $(INC)/pwd.h
bkhistory.o: $(INC)/grp.h
bkhistory.o: $(INC)/table.h
bkhistory.o: $(GENERAL)/bkhist.h
bkhistory.o: $(GENERAL)/bktypes.h
bkhistory.o: $(GENERAL)/backup.h
bkhistory.o: $(GENERAL)/brtoc.h
bkhistory.o: $(LOCAL)/errors.h
