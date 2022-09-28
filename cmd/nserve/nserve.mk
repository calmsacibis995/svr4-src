#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nserve:nserve.mk	1.11.14.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/rfs
INC = $(ROOT)/usr/include
SYMLINK = :
INS = install
CFLAGS=-O -s $(DEBUG) $(LOG) $(PROFILE)
CC = cc
NSLIB = -lns
NSL = -lnsl_s
LIBS = $(NSLIB) $(NSL) -lcrypt_i
LLIB = $(ROOT)/usr/src/lib/libns/llib-lns.ln
LOG=-DLOGGING -DLOGMALLOC
PROFILE=
DEBUG=
EXECS=TPnserve nserve
SOURCE=TPnserve.c nsrec.c nsfunc.c nsdb.c
OBJECTS=TPnserve.o nsrec.o nsfunc.o nsdb.o
SWSRC=nserve.c
SWOBJ=nserve.o
FRC =

all:
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(MAKE) -f nserve.mk all2 ; \
	else \
		$(MAKE) -f nserve.mk NSL=-lnsl all2 ; \
	fi

all2:	$(EXECS)

nserve:	$(SWOBJ)
	$(CC) $(CFLAGS) $(SWOBJ) $(LIBS) -o $(TESTDIR)/nserve $(SHLIBS)

TPnserve: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(TESTDIR)/TPnserve $(SHLIBS)
debug:
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(MAKE) -f nserve.mk DEBUG="-g -DLOGGING -DLOGMALLOC" all2 ; \
	else \
		$(MAKE) -f nserve.mk NSL=-lnsl DEBUG="-g -DLOGGING -DLOGMALLOC" all2 ; \
	fi
dashg:
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(MAKE) -f nserve.mk NSL=-lnsl NSLIB=-lnsdb DEBUG="-g -DLOGGING -DLOGMALLOC" all2 ; \
	else \
		$(MAKE) -f nserve.mk NSL="-Bstatic -lnsl -Bdynamic" NSLIB=-lnsdb DEBUG="-g -DLOGGING -DLOGMALLOC" all2 ; \
	fi
lint:
	lint -pua $(SOURCE) $(LLIB)
	lint -pua $(SWSRC) $(LLIB)

install: all $(INSDIR)
	-rm -f $(ROOT)/usr/nserve/TPnserve
	-rm -f $(ROOT)/usr/nserve/nserve
	$(INS) -f $(INSDIR) -m 0550 -u bin -g bin nserve
	$(INS) -f $(INSDIR) -m 0550 -u bin -g bin TPnserve
	-$(SYMLINK) /usr/lib/rfs/TPnserve $(ROOT)/usr/nserve/TPnserve
	-$(SYMLINK) /usr/lib/rfs/nserve $(ROOT)/usr/nserve/nserve

$(INSDIR):
	mkdir $@
	$(CH)chmod 775 $@
	$(CH)chgrp sys $@
	$(CH)chown root $@
uninstall:
	(cd $(INSDIR); rm -f $(EXECS))

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(EXECS)
FRC: 

#### dependencies now follow

nserve.o: nsdb.h nslog.h $(INC)/nsaddr.h stdns.h $(INC)/nserve.h nsports.h
nsrec.o: nsdb.h nslog.h $(INC)/nsaddr.h stdns.h $(INC)/nserve.h nsports.h
nsdb.o: nsdb.h stdns.h nslog.h
nsfunc.o: nsdb.h stdns.h $(INC)/nserve.h nslog.h
nsswitch.o: nsdb.h nslog.h $(INC)/nsaddr.h stdns.h $(INC)/nserve.h
