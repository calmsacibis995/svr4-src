#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libpkg:libpkg3.2.mk	1.1.2.1"

AR=ar
CC=cc
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LIBPKG=libpkg3.2.a
INSTALL=install
PRESVR4=-D PRESVR4 -D _STYPES -U __STDC__
LINTFILES=srchcfile.c putcfile.c \
	gpkgmap.c ppkgmap.c tputcfent.c \
	verify.c cvtpath.c mappath.c \
	canonize.c logerr.c progerr.c \
	dstream.c pkgtrans.c \
	gpkglist.c isdir.c runcmd.c \
	rrmdir.c ckvolseq.c devtype.c \
	pkgmount.c pkgexecv.c pkgexecl.c 

CFLAGS=-O -I /usr/include -I $(INC) $(PRESVR4)

PKGINFO_FILES=\
	$(LIBPKG)(srchcfile.o) $(LIBPKG)(putcfile.o) \
	$(LIBPKG)(gpkgmap.o) $(LIBPKG)(ppkgmap.o) $(LIBPKG)(tputcfent.o)\
	$(LIBPKG)(verify.o) $(LIBPKG)(cvtpath.o) $(LIBPKG)(mappath.o) \
	$(LIBPKG)(canonize.o) $(LIBPKG)(logerr.o) $(LIBPKG)(progerr.o) \
	$(LIBPKG)(dstream.o) $(LIBPKG)(pkgtrans.o) \
	$(LIBPKG)(gpkglist.o) $(LIBPKG)(isdir.o) $(LIBPKG)(runcmd.o) \
	$(LIBPKG)(rrmdir.o) $(LIBPKG)(ckvolseq.o) $(LIBPKG)(devtype.o) \
	$(LIBPKG)(pkgmount.o) \
	$(LIBPKG)(pkgexecv.o) $(LIBPKG)(pkgexecl.o) $(LIBPKG)(stubs.o)

all:	$(LIBPKG)

.PRECIOUS: $(LIBPKG)

$(LIBPKG): $(PKGINFO_FILES) 

$(PKGINFO_FILES): $(INC)/pkginfo.h $(INC)/pkgstrct.h

clean:

clobber: clean
	rm -f $(LIBPKG)

strip:

install: all
	$(INSTALL) -f $(USRLIB) $(LIBPKG)

lintit:
