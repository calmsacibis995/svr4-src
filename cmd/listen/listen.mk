#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)listen:listen.mk	1.14.10.1"
#
# listen.mk: makefile for network listener 
#

INC	= $(ROOT)/usr/include
INCSYS	= $(ROOT)/usr/include
OPT	= -O -s
# if debug is needed then add -DDEBUGMODE to following line
CFLAGS	= -I$(INC) -I$(INCSYS) ${OPT} 
LDFLAGS	=
LSTINC	= .
LINT = lint
LINTFLAGS = -b -x

# change the next line to compile with -g
# OPT	= -g

INSDIR = $(ROOT)/usr/lib/saf

LSUID	= root
LSGID	= sys
LIBID	= bin
INCID	= bin


# The DEBUG module can always be included...
# if DEBUGMODE is undefined, no code gets compiled.
# doprnt.o is from system V rel 5.0.5.

DBGOBJ	= doprnt.o
DBGSRC	= doprnt.c

SRC	= \
	listen.c \
	lsdata.c \
	lsdbf.c \
	lslog.c \
	nlsaddr.c \
	nstoa.c \
	$(DBGSRC)

NLPSSRC = \
	nlps_serv.c \
	lsdbf.c \
	lssmb.c \
	lsdata.c \
	lslog.c \
	nstoa.c \
	$(DBGSRC)

PRODUCT	= $(INSDIR)/listen

LSTINCS = \
	$(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/varargs.h \
	$(INC)/string.h \
	$(INC)/errno.h \
	$(INCSYS)/sys/utsname.h \
	$(INCSYS)/sys/tiuser.h \
	$(INCSYS)/sys/poll.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stat.h \
	$(INC)/values.h \
	$(INC)/ctype.h \
	$(INC)/pwd.h \
	$(INC)/grp.h \
	$(INC)/utmp.h \
	$(INC)/sac.h \
	$(INC)/time.h \
	$(LSTINC)/lsparam.h \
	$(LSTINC)/lsfiles.h \
	$(LSTINC)/lserror.h \
	$(LSTINC)/lsnlsmsg.h \
	$(LSTINC)/lssmbmsg.h \
	$(LSTINC)/lsdbf.h

DBGINCS	= \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/varargs.h \
	$(INC)/values.h \
	$(LSTINC)/lsparam.h \
	$(LSTINC)/print.h

SMBINCS = \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(LSTINC)/lssmbmsg.h \
	$(LSTINC)/lsparam.h \
	$(INCSYS)/sys/tiuser.h \
	$(LSTINC)/lsdbf.h

CONINCS = \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/fcntl.h \
	$(INC)/errno.h \
	$(INCSYS)/sys/utsname.h \
	$(INCSYS)/sys/tiuser.h \
	$(INC)/listen.h

ALLINCS = $(LSTINCS) $(DBGINCS) $(SMBINCS) $(CONINCS)

LSOBJS	= \
	listen.o \
	lslog.o \
	lsdbf.o \
	lsdata.o \
	nstoa.o \
	nlsaddr.o

NLPSOBJS = \
	nlps_serv.o \
	lsdbf.o \
	lssmb.o \
	nstoa.o \
	lslog.o \
	lsdata.o

all:	listen nlps_server

# 
# SHAREDLIB version
#

listen:		$(LSOBJS) $(DBGOBJ)
		if [ x$(CCSTYPE) = xCOFF ] ; \
		then \
			$(CC) $(CFLAGS) -o listen $(LSOBJS) $(DBGOBJ) $(LDFLAGS) -lnsl_s $(SHLIBS) ; \
		else \
			$(CC) $(CFLAGS) -o listen $(LSOBJS) $(DBGOBJ) $(LDFLAGS) -lnsl $(SHLIBS) ; \
		fi

nlps_server:	$(NLPSOBJS) $(DBGOBJ)
		if [ x$(CCSTYPE) = xCOFF ] ; \
		then \
			$(CC) $(CFLAGS) -o nlps_server $(NLPSOBJS) $(DBGOBJ) $(LDFLAGS) -lnsl_s $(SHLIBS) ; \
		else \
			$(CC) $(CFLAGS) -o nlps_server $(NLPSOBJS) $(DBGOBJ) $(LDFLAGS) -lnsl $(SHLIBS) ; \
		fi

lintit:
		$(LINT) $(LINTFLAGS) $(SRC)
		$(LINT) $(LINTFLAGS) $(NLPSSRC)

nlps_serv.o:	$(INC)/unistd.h $(INCSYS)/sys/mkdev.h $(LSTINCS)
listen.o:	$(INC)/unistd.h $(INCSYS)/sys/mkdev.h $(LSTINCS)
lsdbf.o:	$(LSTINCS)
lslog.o:	$(LSTINCS)
lssmb.o:	$(INC)/stdio.h $(INC)/string.h $(LSTINC)/lssmbmsg.h \
		$(LSTINC)/lsparam.h $(INCSYS)/sys/tiuser.h $(LSTINC)/lsdbf.h
lsdata.o:	$(LSTINCS)
nlsaddr.o:	$(INC)/ctype.h $(INCSYS)/sys/tiuser.h
doprnt.o:	$(DBGINCS)


install:	all $(INSDIR)
		install -o -f $(INSDIR) -u $(LSUID) -g $(LSGID) listen
		install -f $(INSDIR) -u $(LSUID) -g $(LSGID) nlps_server

$(INSDIR):
		mkdir $@
		$(CH)chown $(LSUID) $@
		$(CH)chgrp $(LSGID) $@

clean:
	-rm -f *.o

clobber: clean
	-rm -f listen
	-rm -f nlps_server

FRC:
