#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rslib.d/makefile	1.9.2.1"

SRC1= rsspawn.c rsstrat.c rstm.c rstree.c queue.c rsfile.c gethdr.c
OBJ=$(SRC1:.c=.o)

INC=$(ROOT)/usr/include
BKRSDIR=$(ROOT)/usr/oam/bkrs
BKBIN=$(ROOT)/usr/sadm/bkup/bin
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(ROOT)/sbin
USBIN=$(ROOT)/usr/sbin
ETCBKUP=$(ROOT)/etc/bkup
CC=$(PFX)cc
AR=ar
LFLAGS=-u
LORDER=$(PFX)lorder
LINT=$(PFX)lint

BKLIB=../bklib.d/bklib.a
LIBBR=../libbrmeth.d/libbrmeth.a
IOLIB=../libadmIO.d/libadmIO.a
LOCAL=hdrs
GENERAL=../hdrs
FLAGS=
LDFLAGS=$(LDLIBPATH)
CFLAGS=-I . -I $(LOCAL) -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=rslib.a
TABLES=rsmethod.tab rsstrat.tab

all: $(PRODUCT)

rslib.a: $(OBJ)
	$(AR) cr $(@) `$(LORDER) $(OBJ) | tsort`

touch: 
	touch $(SRC1)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS) $(SRC1)

strip: $(PRODUCT)

install: $(PRODUCT) $(TABLES)
	install -f $(ETCBKUP) -u root -g sys -m 0644 rsmethod.tab
	install -f $(ETCBKUP) -u root -g sys -m 0644 rsstrat.tab

rstest: $(PRODUCT) $(BKLIB) $(LIBBR) $(IOLIB) rstest.o
	cc $(CFLAGS) -o $(@) rstest.o $(PRODUCT) $(BKLIB) $(LIBBR) $(IOLIB) -lTL -lgen

rsspawn.o: $(INC)/stdio.h
rsspawn.o: $(INC)/signal.h
rsspawn.o: $(INC)/sys/types.h
rsspawn.o: $(INC)/errno.h
rsspawn.o: $(INC)/bkrs.h
rsspawn.o: $(GENERAL)/backup.h
rsspawn.o: $(GENERAL)/restore.h
rsspawn.o: $(GENERAL)/rsstatus.h
rsspawn.o: $(GENERAL)/bkerrors.h
rsstrat.o: $(INC)/fcntl.h
rsstrat.o: $(INC)/time.h
rsstrat.o: $(INC)/errno.h
rsstrat.o: $(INC)/table.h
rsstrat.o: $(GENERAL)/bktypes.h
rsstrat.o: $(GENERAL)/bkerrors.h
rsstrat.o: $(GENERAL)/restore.h
rsstrat.o: $(GENERAL)/rsmethod.h
rsstrat.o: $(GENERAL)/rsstrat.h
rsstrat.o: $(LOCAL)/rstm.h
rstest.o: $(INC)/time.h
rstest.o: $(INC)/fcntl.h
rstest.o: $(INC)/stdio.h
rstest.o: $(INC)/ctype.h
rstest.o: $(INC)/table.h
rstest.o: $(INC)/errno.h
rstest.o: $(INC)/bkrs.h
rstest.o: $(GENERAL)/restore.h
rstest.o: $(GENERAL)/bkhist.h
rstm.o: $(INC)/fcntl.h
rstm.o: $(GENERAL)/brtoc.h
rstm.o: $(INC)/sys/types.h
rstm.o: $(INC)/sys/stat.h
rstm.o: $(INC)/time.h
rstm.o: $(INC)/table.h
rstm.o: $(GENERAL)/bktypes.h
rstm.o: $(GENERAL)/bkhist.h
rstm.o: $(GENERAL)/restore.h
rstm.o: $(GENERAL)/rsstrat.h
rstm.o: $(GENERAL)/rsmethod.h
rstm.o: $(LOCAL)/rstm.h
rstree.o: $(INC)/search.h
rstree.o: $(GENERAL)/bktypes.h
rstree.o: $(GENERAL)/bkerrors.h
rstree.o: $(LOCAL)/rstm.h
rsspawn.o: $(INC)/stdio.h
rsspawn.o: $(INC)/sys/types.h
rsspawn.o: $(INC)/errno.h
rsspawn.o: $(INC)/bkrs.h
rsspawn.o: $(GENERAL)/backup.h
rsspawn.o: $(GENERAL)/bkmsgs.h
rsspawn.o: $(GENERAL)/restore.h
rsspawn.o: $(GENERAL)/bkerrors.h
queue.o: $(INC)/sys/types.h
queue.o: $(INC)/time.h
queue.o: $(GENERAL)/backup.h
queue.o: $(GENERAL)/bkmsgs.h
rsfile.o: $(INC)/sys/types.h
rsfile.o: $(INC)/sys/stat.h
rsfile.o: $(INC)/stdio.h
rsfile.o: $(INC)/sys/mnttab.h
rsfile.o: $(INC)/errno.h
gethdr.o: $(INC)/time.h
gethdr.o: $(INC)/string.h
gethdr.o: $(INC)/pwd.h
gethdr.o: $(GENERAL)/bktypes.h
gethdr.o: $(GENERAL)/brarc.h
gethdr.o: $(INC)/bkrs.h
gethdr.o: $(GENERAL)/libadmIO.h
