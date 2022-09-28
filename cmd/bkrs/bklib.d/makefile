#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:bklib.d/makefile	1.13.2.1"

SRC1=bkparse.c bkdates.c bkrotate.c bkutils.c jobidtopid.c format.c brerrno.c
SRC2=bkerror.c bkspawn.c bkpath.c brgetdate.c bkstrtok.c bkstrdup.c bld_ddev.c
SRC3=bkstrcmp.c bkstrncpy.c bkrot.c bkprint.c bkisjob.c bkgetchars.c device.c
OBJ=$(SRC1:.c=.o) $(SRC2:.c=.o) $(SRC3:.c=.o)

INC=$(ROOT)/usr/include
GDATE=$(ROOT)/usr/lib/getdate
CC=$(PFX)cc
AR=ar
LFLAGS=-u
LORDER=$(PFX)lorder
LINT=$(PFX)lint

GENERAL=../hdrs
FLAGS=
CFLAGS=-I . -I $(GENERAL) -I $(INC) $(FLAGS)

PRODUCT=bklib.a bkrsdates

all: $(PRODUCT)

bklib.a: $(OBJ)
	$(AR) cr $(@) `$(LORDER) $(OBJ) | tsort`

bkrsdates: bkrsdates.dat
	grep -v "^#ident"  bkrsdates.dat >bkrsdates; chmod 444 bkrsdates

touch: 
	touch $(SRC1) $(SRC2) $(SRC3)

clean:
	rm -f $(OBJ) 

clobber: clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) -u $(LFLAGS) $(CFLAGS) $(SRC1) $(SRC2) $(SRC3)

install: $(PRODUCT)
	install -f $(GDATE) -u bin -g bin -m 444 bkrsdates

strip: $(PRODUCT)

bkdates.o: $(INC)/sys/types.h
bkdates.o: $(INC)/time.h
bkdates.o: $(INC)/stdio.h
bkdates.o: $(GENERAL)/backup.h
bkerror.o: $(INC)/stdio.h
bkerror.o: $(INC)/varargs.h
bkparse.o: $(INC)/ctype.h
bkparse.o: $(INC)/stdio.h
bkparse.o: $(GENERAL)/backup.h
bkpath.o: $(INC)/string.h
bkpath.o: $(GENERAL)/backup.h
bkrotate.o: $(INC)/stdio.h
bkrotate.o: $(GENERAL)/backup.h
bkrotate.o: $(INC)/table.h
bkspawn.o: $(INC)/fcntl.h
bkspawn.o: $(INC)/sys/types.h
bkspawn.o: $(INC)/varargs.h
bkspawn.o: $(INC)/errno.h
bkspawn.o: $(GENERAL)/backup.h
bkutils.o: $(INC)/stdio.h
bkutils.o: $(INC)/ctype.h
bkutils.o: $(INC)/errno.h
bkutils.o: $(INC)/table.h
brerrno.o: $(INC)/errno.h
format.o: $(INC)/table.h
format.o: $(GENERAL)/bkreg.h
bkstrtok.o: $(INC)/sys/param.h
bkstrtok.o: $(INC)/sys/types.h
bkrot.o: $(INC)/table.h
bkrot.o: $(INC)/stdio.h
bkrot.o: $(INC)/string.h
bkrot.o: $(GENERAL)/bkerrors.h
bkstrncpy.o: $(INC)/sys/types.h
bkstrncpy.o: $(INC)/string.h
bkprint.o: $(INC)/stdio.h
bkprint.o: $(INC)/table.h
bkisjob.o: $(INC)/string.h
bkisjob.o: $(INC)/ctype.h
device.o: $(INC)/sys/types.h
device.o: $(GENERAL)/bktypes.h
device.o: $(GENERAL)/backup.h
device.o: $(GENERAL)/bkerrors.h
device.o: $(INC)/devmgmt.h
device.o: $(INC)/errno.h
jobidtopid.o: $(INC)/sys/types.h
