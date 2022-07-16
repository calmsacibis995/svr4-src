#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libadm:libadm3.2.mk	1.1.2.1"

CC=cc
AR=ar
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
INSTALL=install
LIBADM=libadm3.2.a
OBJECTS=$(LIBADM)

LINTFILES= \
	pkginfo.c pkgnmchk.c pkgparam.c \
	getinput.c ckint.c ckitem.c \
	ckpath.c ckrange.c ckstr.c \
	ckyorn.c puterror.c puthelp.c \
	puttext.c ckkeywd.c getvol.c \
	devattr.c putprmpt.c ckgid.c \
	ckdate.c cktime.c ckuid.c \
	dgrpent.c getdev.c \
	devtab.c data.c getdgrp.c \
	listdev.c listdgrp.c regexp.c \
	devreserv.c putdev.c putdgrp.c \
	memory.c

CFLAGS=-I . -I/usr/include -I $(INC) $(XFLAGS) $(CCFLAGS) -D PRESVR4 -D _STYPES -U __STDC__ -O

PKGINFO_FILES=\
	$(LIBADM)(pkginfo.o) $(LIBADM)(pkgnmchk.o) $(LIBADM)(pkgparam.o) 

VALTOOL_FILES=\
	$(LIBADM)(getinput.o) $(LIBADM)(ckint.o) $(LIBADM)(ckitem.o) \
	$(LIBADM)(ckpath.o) $(LIBADM)(ckrange.o) $(LIBADM)(ckstr.o) \
	$(LIBADM)(ckyorn.o) $(LIBADM)(puterror.o) $(LIBADM)(puthelp.o) \
	$(LIBADM)(puttext.o) $(LIBADM)(ckkeywd.o) $(LIBADM)(getvol.o) \
	$(LIBADM)(devattr.o) $(LIBADM)(putprmpt.o) $(LIBADM)(ckgid.o) \
	$(LIBADM)(ckdate.o) $(LIBADM)(cktime.o) $(LIBADM)(ckuid.o) \
	$(LIBADM)(space.o)

DEVMGMT_FILES=\
	$(LIBADM)(dgrpent.o) $(LIBADM)(getdev.o) \
	$(LIBADM)(devtab.o) $(LIBADM)(data.o) $(LIBADM)(getdgrp.o) \
	$(LIBADM)(listdev.o) $(LIBADM)(listdgrp.o) $(LIBADM)(regexp.o) \
	$(LIBADM)(devreserv.o) $(LIBADM)(putdev.o) $(LIBADM)(putdgrp.o) \
	$(LIBADM)(memory.o) 



all: $(LIBADM)

$(PKGINFO_FILES):	$(INC)/pkginfo.h $(INC)/pkgstrct.h

.PRECIOUS: $(LIBADM)

$(LIBADM): $(PKGINFO_FILES) $(DEVMGMT_FILES) $(VALTOOL_FILES)

clean:

clobber: clean
	rm -f $(LIBADM)

strip:

install: all 
	$(INSTALL) -f $(USRLIB) $(LIBADM)

lintit:
