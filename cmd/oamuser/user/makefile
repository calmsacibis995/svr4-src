#ident	"@(#)makefile	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamuser:user/makefile	1.9.5.1"



INC=$(ROOT)/usr/include
LOCAL=../inc
ETC=$(ROOT)/etc
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LBINDIR=$(ROOT)/usr/sbin
BINDIR=$(ROOT)/usr/sadm/sysadm/bin
SKELDIR=$(ROOT)/etc/skel
HERE=.

CC=$(PFX)cc
MAKE=make
STRIP=$(PFX)strip
LINT=$(PFX)lint
SIZE=size
INS=install

LIBDIR = ../lib
ADMLIB = $(USRLIB)/libadm.a
LIBUSRGRP=$(LIBDIR)/lib.a

FLAGS=-O
LDFLAGS=$(LDLIBPATH) -s
CFLAGS=-I$(LOCAL) -I$(INC) $(FLAGS)
LFLAGS=-u

PRODUCTS=useradd userdel usermod getusrdefs putusrdefs getngroups \
	finduid .profile expdate

ADD_SRC=useradd.c putusrdefs.c uid.c homedir.c groups.c \
	call_pass.c userdefs.c messages.c val_lgrp.c
DEL_SRC=userdel.c call_pass.c rmfiles.c isbusy.c groups.c \
	messages.c 
MOD_SRC=usermod.c uid.c movedir.c groups.c rmfiles.c \
	call_pass.c isbusy.c homedir.c userdefs.c messages.c val_lgrp.c
GETDEFS_SRC=getdefs.c userdefs.c
PUTDEF_SRC=putdefs.c putusrdefs.c userdefs.c messages.c
GETNGR_SRC=getngroups.c
FINDUIDSRC=finduid.c uid.c
EXPDSRC=expdate.c

ADD_OBJ=$(ADD_SRC:.c=.o)
DEL_OBJ=$(DEL_SRC:.c=.o)
MOD_OBJ=$(MOD_SRC:.c=.o)

GETDEFS_OBJ=$(GETDEFS_SRC:.c=.o)
PUTDEFS_OBJ=$(PUTDEF_SRC:.c=.o)
FINDUIDOBJ=$(FINDUIDSRC:.c=.o)

GETNGR_OBJ=$(GETNGR_SRC:.c=.o)
EXPDOBJ=$(EXPDSRC:.c=.o)

OBJECTS = $(ADD_OBJ) $(DEL_OBJ) $(MOD_OBJ) $(GETDEFS_OBJ) \
		$(PUTDEFS_OBJ) $(GETNGR_OBJ) $(FINDUIDOBJ) $(EXPDOBJ)

all: $(PRODUCTS)

useradd: $(ADD_OBJ) $(LIBUSRGRP) $(ADMLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(ADD_OBJ) $(LIBUSRGRP) \
		-lgen -lelf $(ADMLIB) $(LDLIBS)

userdel: $(DEL_OBJ) $(LIBUSRGRP) $(ADMLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(DEL_OBJ) $(LIBUSRGRP) -lelf $(LDLIBS)

usermod: $(MOD_OBJ) $(LIBUSRGRP) $(ADMLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(MOD_OBJ) $(LIBUSRGRP) \
		-lgen -lelf $(LDLIBS)

getusrdefs: $(GETDEFS_OBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(GETDEFS_OBJ) $(LIBUSRGRP) -lelf $(LDLIBS)

putusrdefs: $(PUTDEFS_OBJ) $(LIBUSRGRP) $(ADMLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(PUTDEFS_OBJ) $(LIBUSRGRP) \
		-lgen -lelf $(ADMLIB) $(LDLIBS)

getngroups: $(GETNGR_OBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(GETNGR_OBJ) $(LIBUSRGRP) -lelf $(LDLIBS)

finduid: $(FINDUIDOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(FINDUIDOBJ) -lelf $(LDLIBS)

.profile: stdprofile
	grep -v "^#ident" stdprofile > .profile

expdate: $(EXPDOBJ) $(LIBUSRGRP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(EXPDOBJ) $(LIBUSRGRP) -lgen -lelf $(LDLIBS)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)

install: all
	-[ -d $(LBINDIR) ] || mkdir -p $(LBINDIR)
	-[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	-[ -d $(SKELDIR) ] || mkdir -p $(SKELDIR)
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) useradd
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) userdel
	$(INS) -m 500 -g sys -u root -f $(LBINDIR) usermod
	$(INS) -m 500 -g sys -u root -f $(BINDIR) getusrdefs
	$(INS) -m 500 -g sys -u root -f $(BINDIR) putusrdefs
	$(INS) -m 500 -g sys -u root -f $(BINDIR) getngroups
	$(INS) -m 500 -g sys -u root -f $(BINDIR) expdate
	$(INS) -m 500 -g sys -u root -f $(BINDIR) finduid
	$(INS) -m 644 -g other -u root -f $(SKELDIR) .profile

size: all
	$(SIZE) $(PRODUCTS)

strip: all
	$(STRIP) $(PRODUCTS)

lintit:
	$(LINT) $(LFLAGS) $(CFLAGS) $(ADD_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(DEL_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(MOD_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(GETDEFS_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(PUTDEF_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(GETNGR_SRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(FINDUIDSRC)
	$(LINT) $(LFLAGS) $(CFLAGS) $(EXPDSRC)

call_pass.o: $(INC)/sys/types.h
call_pass.o: $(INC)/stdio.h
call_pass.o: $(INC)/userdefs.h
groups.o: $(INC)/sys/types.h
groups.o: $(INC)/stdio.h
groups.o: $(INC)/ctype.h
groups.o: $(INC)/grp.h
groups.o: $(INC)/unistd.h
groups.o: $(INC)/userdefs.h
groups.o: $(LOCAL)/users.h
rmfiles.o: $(INC)/sys/types.h
rmfiles.o: $(INC)/stdio.h
rmfiles.o: $(INC)/userdefs.h
rmfiles.o: $(INC)/errno.h
rmfiles.o: $(HERE)/messages.h
userdel.o: $(INC)/sys/types.h
userdel.o: $(INC)/sys/stat.h
userdel.o: $(INC)/stdio.h
userdel.o: $(INC)/ctype.h
userdel.o: $(INC)/limits.h
userdel.o: $(INC)/pwd.h
userdel.o: $(INC)/string.h
userdel.o: $(INC)/sys/types.h
userdel.o: $(INC)/sys/stat.h
userdel.o: $(INC)/userdefs.h
userdel.o: $(INC)/errno.h
userdel.o: $(LOCAL)/users.h
userdel.o: $(HERE)/messages.h
finduid.o: $(INC)/sys/types.h
finduid.o: $(INC)/stdio.h
finduid.o: $(INC)/userdefs.h
homedir.o: $(INC)/errno.h
homedir.o: $(INC)/sys/types.h
homedir.o: $(INC)/stdio.h
homedir.o: $(INC)/userdefs.h
homedir.o: $(HERE)/messages.h
movedir.o: $(INC)/stdio.h
movedir.o: $(INC)/sys/types.h
movedir.o: $(INC)/userdefs.h
movedir.o: $(HERE)/messages.h
uid.o: $(INC)/sys/types.h
uid.o: $(INC)/stdio.h
uid.o: $(INC)/userdefs.h
uid.o: $(INC)/sys/param.h
uid.o: $(INC)/limits.h
usermod.o: $(INC)/sys/types.h
usermod.o: $(INC)/sys/stat.h
usermod.o: $(INC)/sys/param.h
usermod.o: $(INC)/stdio.h
usermod.o: $(INC)/ctype.h
usermod.o: $(INC)/limits.h
usermod.o: $(INC)/pwd.h
usermod.o: $(INC)/grp.h
usermod.o: $(INC)/string.h
usermod.o: $(INC)/userdefs.h
usermod.o: $(LOCAL)/users.h
usermod.o: $(HERE)/messages.h
getdefs.o: $(INC)/sys/types.h
getdefs.o: $(INC)/stdio.h
getdefs.o: $(INC)/userdefs.h
getdefs.o: $(HERE)/userdisp.h
isbusy.o: $(INC)/stdio.h
isbusy.o: $(INC)/string.h
isbusy.o: $(INC)/sys/types.h
isbusy.o: $(INC)/utmp.h
putdefs.o: $(INC)/stdio.h
putdefs.o: $(INC)/ctype.h
putdefs.o: $(INC)/limits.h
putdefs.o: $(INC)/grp.h
putdefs.o: $(INC)/sys/types.h
putdefs.o: $(INC)/sys/stat.h
putdefs.o: $(INC)/userdefs.h
putdefs.o: $(LOCAL)/users.h
putdefs.o: $(HERE)/messages.h
useradd.o: $(INC)/sys/types.h
useradd.o: $(INC)/sys/stat.h
useradd.o: $(INC)/sys/param.h
useradd.o: $(INC)/stdio.h
useradd.o: $(INC)/ctype.h
useradd.o: $(INC)/limits.h
useradd.o: $(INC)/grp.h
useradd.o: $(INC)/string.h
useradd.o: $(INC)/userdefs.h
useradd.o: $(LOCAL)/users.h
useradd.o: $(HERE)/messages.h
useradd.o: $(HERE)/userdisp.h
val_lgrp.o: $(INC)/sys/types.h
val_lgrp.o: $(INC)/stdio.h
val_lgrp.o: $(INC)/sys/param.h
val_lgrp.o: $(INC)/grp.h
val_lgrp.o: $(LOCAL)/users.h
val_lgrp.o: $(INC)/userdefs.h
val_lgrp.o: $(HERE)/messages.h
getngroups.o: $(INC)/stdio.h
putusrdefs.o: $(INC)/stdio.h
putusrdefs.o: $(INC)/time.h
putusrdefs.o: $(INC)/unistd.h
putusrdefs.o: $(INC)/sys/types.h
putusrdefs.o: $(INC)/userdefs.h
putusrdefs.o: $(HERE)/messages.h
userdefs.o: $(INC)/stdio.h
userdefs.o: $(INC)/string.h
userdefs.o: $(INC)/userdefs.h
userdefs.o: $(INC)/limits.h
userdefs.o: $(HERE)/userdisp.h
expdate.o: $(INC)/sys/types.h
expdate.o: $(INC)/stdio.h
expdate.o: $(INC)/userdefs.h
expdate.o: $(LOCAL)/users.h
