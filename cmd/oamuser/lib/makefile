#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamuser:lib/makefile	1.7.5.1"



SRC1=putgrent.c errmsg.c file.c vgid.c vgname.c vgroup.c vuid.c \
	getkval.c get_ngm.c vlogin.c dates.c vexpire.c prerrno.c copyfile.c
OBJ=$(SRC1:.c=.o)

INC=$(ROOT)/usr/include
CC=$(PFX)cc
AR=ar
LFLAGS=-u
LORDER=$(PFX)lorder
LINT=$(PFX)lint

GENERAL=../inc
FLAGS=-O
CFLAGS=-I. -I$(GENERAL) -I$(INC) $(FLAGS)

GDATE=$(ROOT)/usr/sadm

PRODUCT=lib.a ugdates

all: $(PRODUCT)

lib.a: $(OBJ)
	$(AR) cr $(@) `$(LORDER) $(OBJ) | tsort`

ugdates: ugdates.dat
	grep -v "^#ident" ugdates.dat > ugdates
touch: 
	touch $(SRC1) 

clean:
	rm -f $(OBJ) 

clobber: clean
	rm -f $(PRODUCT)

lintit:
	$(LINT) $(LFLAGS) $(CFLAGS) $(SRC1)

install: $(PRODUCT)
	install -f $(GDATE) -u bin -g bin -m 444 ugdates

strip: $(PRODUCT)

size: $(PRODUCT)
	size $(PRODUCT)

errmsg.o: $(INC)/stdio.h
errmsg.o: $(INC)/varargs.h
file.o: $(INC)/sys/stat.h
file.o: $(INC)/sys/types.h
get_ngm.o: $(INC)/sys/param.h
getkval.o: $(INC)/fcntl.h
getkval.o: $(INC)/nlist.h
putgrent.o: $(INC)/stdio.h
putgrent.o: $(INC)/grp.h
putgrent.o: $(INC)/unistd.h
vgid.o: $(INC)/sys/types.h
vgid.o: $(INC)/stdio.h
vgid.o: $(INC)/grp.h
vgid.o: $(GENERAL)/users.h
vgid.o: $(INC)/sys/param.h
vgid.o: $(INC)/userdefs.h
vgid.o: $(INC)/limits.h
vgname.o: $(INC)/sys/types.h
vgname.o: $(INC)/stdio.h
vgname.o: $(INC)/ctype.h
vgname.o: $(INC)/grp.h
vgname.o: $(INC)/userdefs.h
vgname.o: $(GENERAL)/users.h
vgroup.o: $(INC)/sys/types.h
vgroup.o: $(INC)/stdio.h
vgroup.o: $(INC)/ctype.h
vgroup.o: $(INC)/grp.h
vgroup.o: $(GENERAL)/users.h
vuid.o: $(INC)/sys/types.h
vuid.o: $(INC)/stdio.h
vuid.o: $(INC)/pwd.h
vuid.o: $(INC)/userdefs.h
vuid.o: $(GENERAL)/users.h
vuid.o: $(INC)/sys/param.h
vuid.o: $(INC)/limits.h
vlogin.o: $(INC)/sys/types.h
vlogin.o: $(INC)/stdio.h
vlogin.o: $(INC)/ctype.h
vlogin.o: $(INC)/pwd.h
vlogin.o: $(INC)/userdefs.h
vlogin.o: $(GENERAL)/users.h
dates.o: $(INC)/time.h
prerrno.o: $(INC)/errno.h
