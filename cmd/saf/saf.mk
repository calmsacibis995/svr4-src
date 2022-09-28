#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


# ident	"@(#)saf:saf.mk	1.6"

#
# saf.mk: makefile for the Service Access Facility
#

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
OPT = -O
# if debug is needed then add -DDEBUG to following line
CFLAGS = -I$(INC) -I$(INCSYS) $(OPT)
LDFLAGS = $(LLDFLAGS)
ELFNSL = -lnsl
COFFNSL = -lnsl_s
LINT = lint
LINTFLAGS = -b -x
CMDDIR = $(ROOT)/usr/sbin
SACDIR = $(ROOT)/usr/lib/saf
SYMLINK = :
ID = root
GID = sys

# change the next two lines to compile with -g
# OPT = -g
LLDFLAGS = -s

SACSRC = \
	sac.c \
	readtab.c \
	global.c \
	log.c \
	misc.c \
	util.c

SACADMSRC = \
	sacadm.c \
	admutil.c \
	util.c

PMADMSRC = \
	pmadm.c \
	admutil.c \
	util.c

HDR = \
	misc.h \
	sac.h \
	structs.h \
	msgs.h \
	adm.h \
	extern.h

SACOBJ = \
	sac.o \
	readtab.o \
	global.o \
	log.o \
	misc.o \
	util1.o

SACADMOBJ = \
	sacadm.o \
	admutil.o \
	util2.o

PMADMOBJ = \
	pmadm.o \
	admutil.o \
	util2.o

PRODUCTS = sac sacadm pmadm

all:	$(PRODUCTS)

sac:	$(SACOBJ)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) -o sac $(SACOBJ) $(LDFLAGS) $(COFFNSL) $(SHLIBS) ;\
	else \
		$(CC) -o sac $(SACOBJ) $(LDFLAGS) $(ELFNSL) $(SHLIBS) ;\
	fi

sacadm:	$(SACADMOBJ)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) -o sacadm $(SACADMOBJ) $(LDFLAGS) $(COFFNSL) $(SHLIBS) ;\
	else \
		$(CC) -o sacadm $(SACADMOBJ) $(LDFLAGS) $(ELFNSL) $(SHLIBS) ;\
	fi

pmadm:	$(PMADMOBJ)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) -o pmadm $(PMADMOBJ) $(LDFLAGS) $(COFFNSL) $(SHLIBS) ;\
	else \
		$(CC) -o pmadm $(PMADMOBJ) $(LDFLAGS) $(ELFNSL) $(SHLIBS) ;\
	fi

lintit:
	$(LINT) $(LINTFLAGS) $(SACSRC)
	$(LINT) $(LINTFLAGS) $(SACADMSRC)
	$(LINT) $(LINTFLAGS) $(PMADMSRC)

# To share as much code as possible, util.c is compiled into two
# forms, defining SAC for the sac's version of the file and undefining
# it for the administrative commands version

util1.o:	util.c \
		extern.h \
		misc.h \
		msgs.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INC)/unistd.h \
		$(INC)/ctype.h \
		$(INCSYS)/sys/types.h \
		structs.h
	$(CC) -c $(CFLAGS) -DSAC util.c
	mv util.o util1.o

util2.o:	util.c \
		extern.h \
		misc.h \
		structs.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INC)/unistd.h \
		$(INC)/ctype.h \
		$(INCSYS)/sys/types.h
	$(CC) -c $(CFLAGS) -USAC util.c
	mv util.o util2.o

sac.o:	sac.c \
	misc.h \
	structs.h \
	extern.h \
	msgs.h \
	$(INC)/sac.h \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/ctype.h \
	$(INC)/signal.h \
	$(INC)/errno.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/stropts.h \
	$(INC)/unistd.h \
	$(INC)/utmp.h \
	$(INC)/memory.h

readtab.o:	readtab.c \
		misc.h \
		extern.h \
		msgs.h \
		structs.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/types.h \
		$(INC)/unistd.h

log.o:	log.c \
	extern.h \
	misc.h \
	msgs.h \
	structs.h \
	$(INC)/sac.h \
	$(INC)/stdio.h \
	$(INC)/unistd.h \
	$(INCSYS)/sys/types.h

global.o:	global.c \
		misc.h \
		structs.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INCSYS)/sys/types.h

misc.o:	misc.c \
	misc.h \
	structs.h \
	msgs.h \
	extern.h \
	adm.h \
	$(INC)/sac.h \
	$(INC)/stdio.h \
	$(INC)/unistd.h \
	$(INC)/fcntl.h \
	$(INCSYS)/sys/types.h \
	$(INC)/signal.h \
	$(INCSYS)/sys/stat.h \
	$(INC)/poll.h

sacadm.o:	sacadm.c \
		adm.h \
		structs.h \
		misc.h \
		extern.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/errno.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stat.h \
		$(INC)/signal.h \
		$(INC)/unistd.h

pmadm.o:	pmadm.c \
		misc.h \
		extern.h \
		structs.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stat.h \
		$(INC)/unistd.h

admutil.o:	admutil.c \
		misc.h \
		extern.h \
		structs.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stat.h \
		$(INC)/unistd.h \
		$(INC)/sac.h \
		$(INC)/stdio.h \
		$(INC)/signal.h

install:	all $(SACDIR)
	install -o -f $(SACDIR) -u $(ID) -g $(GID) sac
	install -f $(CMDDIR) -u $(ID) -g $(GID) pmadm
	install -f $(CMDDIR) -u $(ID) -g $(GID) -m 4755 sacadm

$(SACDIR):
	mkdir $@

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(PRODUCTS)

FRC:
