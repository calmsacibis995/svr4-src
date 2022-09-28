#ident	"@(#)makefile	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamuser:group/makefile	1.5.6.1"

INC=$(ROOT)/usr/include
LOCAL=../inc
HERE=.
ETC=$(ROOT)/etc
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LBINDIR=$(ROOT)/usr/sbin
BINDIR=$(ROOT)/usr/sadm/sysadm/bin

CC=$(PFX)cc
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint
SIZE=size
INS=install

LIBDIR = ../lib
LIBUSRGRP=$(LIBDIR)/lib.a

FLAGS=-O
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I$(HERE) -I$(LOCAL) -I$(INC) $(FLAGS)
LFLAGS=-u

PRODUCTS=groupadd groupdel groupmod addgrpmem getprimary findgid

ADD_SRC=groupadd.c add_group.c gid.c messages.c
DEL_SRC=groupdel.c del_group.c messages.c
MOD_SRC=groupmod.c mod_group.c messages.c
GRPMEM_SRC=addgrpmem.c
GETPR_SRC=getprimary.c
FINDGIDSRC=findgid.c gid.c

ADD_OBJ=$(ADD_SRC:.c=.o)
DEL_OBJ=$(DEL_SRC:.c=.o)
MOD_OBJ=$(MOD_SRC:.c=.o)

GRPMEM_OBJ=$(GRPMEM_SRC:.c=.o)
GETPR_OBJ=$(GETPR_SRC:.c=.o)
FINDGIDOBJ=$(FINDGIDSRC:.c=.o)

OBJECTS = $(ADD_OBJ) $(DEL_OBJ) $(MOD_OBJ) $(GETMEM_OBJ) $(GETPR_OBJ) \
	$(FINDUIDOBJ) $(GRPMEM_OBJ) $(FINDGIDOBJ)

all: $(PRODUCTS)

groupadd: $(ADD_OBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(ADD_OBJ) $(LIBUSRGRP) $(LDLIBS)

groupdel: $(DEL_OBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(DEL_OBJ) $(LIBUSRGRP) $(LDLIBS)

groupmod: $(MOD_OBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(MOD_OBJ) $(LIBUSRGRP) $(LDLIBS)

addgrpmem: $(GRPMEM_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(GRPMEM_OBJ) $(LIBUSRGRP) $(LDLIBS)

getprimary: $(GETPR_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(GETPR_OBJ) $(LIBUSRGRP) $(LDLIBS)

findgid: $(FINDGIDOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(FINDGIDOBJ) $(LDLIBS)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)

install: all
	-[ -d $(LBINDIR) ] || mkdir -p $(LBINDIR)
	-[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) groupadd
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) groupdel
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) groupmod
	$(INS) -m 500 -g sys -u root -f $(BINDIR) addgrpmem
	$(INS) -m 500 -g sys -u root -f $(BINDIR) getprimary
	$(INS) -m 500 -g sys -u root -f $(BINDIR) findgid

size: all
	$(SIZE) $(PRODUCTS)

strip: all
	$(STRIP) $(PRODUCTS)

lintit:
	$(LINT) $(LFLAGS) $(CFLAGS) $(ADD_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(DEL_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(MOD_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(GRPMEM_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(GETPR_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(FINDGIDSRC)

add_group.o: $(INC)/sys/types.h
add_group.o: $(INC)/stdio.h
add_group.o: $(INC)/unistd.h
add_group.o: $(INC)/userdefs.h
addgrpmem.o: $(INC)/sys/types.h
addgrpmem.o: $(INC)/stdio.h
addgrpmem.o: $(INC)/ctype.h
addgrpmem.o: $(INC)/limits.h
addgrpmem.o: $(INC)/grp.h
addgrpmem.o: $(INC)/pwd.h
addgrpmem.o: $(INC)/errno.h
addgrpmem.o: $(INC)/unistd.h
addgrpmem.o: $(LOCAL)/users.h
addgrpmem.o: $(INC)/userdefs.h
del_group.o: $(INC)/sys/types.h
del_group.o: $(INC)/stdio.h
del_group.o: $(INC)/ctype.h
del_group.o: $(INC)/grp.h
del_group.o: $(INC)/unistd.h
del_group.o: $(INC)/userdefs.h
del_group.o: $(LOCAL)/users.h
del_group.o: $(INC)/errno.h
findgid.o: $(INC)/sys/types.h
findgid.o: $(INC)/stdio.h
findgid.o: $(INC)/userdefs.h
getprimary.o: $(INC)/sys/types.h
getprimary.o: $(INC)/stdio.h
getprimary.o: $(INC)/pwd.h
getprimary.o: $(INC)/grp.h
getprimary.o: $(INC)/userdefs.h
groupadd.o: $(INC)/sys/types.h
groupadd.o: $(INC)/stdio.h
groupadd.o: $(INC)/ctype.h
groupadd.o: $(INC)/limits.h
groupadd.o: $(INC)/userdefs.h
groupadd.o: $(LOCAL)/users.h
groupadd.o: $(HERE)/messages.h
groupdel.o: $(INC)/sys/types.h
groupdel.o: $(INC)/stdio.h
groupdel.o: $(INC)/userdefs.h
groupdel.o: $(HERE)/messages.h
groupmod.o: $(INC)/sys/types.h
groupmod.o: $(INC)/stdio.h
groupmod.o: $(INC)/ctype.h
groupmod.o: $(INC)/limits.h
groupmod.o: $(INC)/grp.h
groupmod.o: $(INC)/userdefs.h
groupmod.o: $(LOCAL)/users.h
groupmod.o: $(HERE)/messages.h
mod_group.o: $(INC)/sys/types.h
mod_group.o: $(INC)/stdio.h
mod_group.o: $(INC)/ctype.h
mod_group.o: $(INC)/grp.h
mod_group.o: $(INC)/unistd.h
mod_group.o: $(INC)/userdefs.h
mod_group.o: $(LOCAL)/users.h
mod_group.o: $(INC)/errno.h
gid.o: $(INC)/sys/types.h
gid.o: $(INC)/stdio.h
gid.o: $(INC)/userdefs.h
gid.o: $(INC)/sys/param.h
gid.o: $(INC)/limits.h
