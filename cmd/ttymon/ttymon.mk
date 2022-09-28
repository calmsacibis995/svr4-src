#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ttymon:ttymon.mk	1.17.5.1"

#
# ttymon.mk: makefile for ttymon and stty
#

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
OPT = -O
# If machine name and /etc/issue file need to be printed 
# before the service prompt is printed, then add -DSYS_NAME to CFLAGS
# If debug is needed then add -DDEBUG to following line
DFLGS = -DMERGE386
CFLAGS = -I$(INC) -I$(INCSYS) $(OPT) -DSYS_NAME $(DFLGS)
LDFLAGS = $(LLDFLAGS)
SHLIBS=
NOSHLIBS=-dn
ELFNSL = -lnsl
COFFNSL = -lnsl_s
LINT = lint
MAKE = make
LINTFLAGS = -b -x
SYMLINK = :
CMDDIR = $(ROOT)/usr/sbin
TTYMONDIR = $(ROOT)/usr/lib/saf
GETTYDIR = $(ROOT)/usr/sbin
INS = install
ID = root
GID = sys

# change the next two lines to compile with -g
# OPT = -g
LLDFLAGS = -s

TTYMONSRC= \
		ttymon.c \
		tmglobal.c \
		tmhandler.c \
		tmpmtab.c \
		tmttydefs.c \
		tmparse.c \
		tmsig.c \
		tmsac.c \
		tmchild.c \
		tmautobaud.c \
		tmterm.c \
		tmutmp.c \
		tmpeek.c \
		tmlog.c \
		tmlock.c \
		tmutil.c \
		tmexpress.c \
		sttytable.c \
		sttyparse.c \
		ulockf.c

TTYADMSRC= \
		ttyadm.c \
		tmutil.c \
		admutil.c 

STTYDEFSSRC= \
		sttydefs.c \
		admutil.c \
		tmttydefs.c \
		tmparse.c \
		sttytable.c \
		sttyparse.c 

HDR = \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		stty.h 

TTYMONOBJ= \
		ttymon.o \
		tmglobal.o \
		tmhandler.o \
		tmpmtab.o \
		tmttydefs.o \
		tmparse.o \
		tmsig.o \
		tmsac.o \
		tmchild.o \
		tmautobaud.o \
		tmterm.o \
		tmutmp.o \
		tmpeek.o \
		tmlog.o \
		tmlock.o \
		tmutil.o \
		tmexpress.o \
		sttytable.o \
		sttyparse.o \
		ulockf.o

TTYADMOBJ= \
		ttyadm.o \
		tmutil.o \
		admutil.o 

STTYDEFSOBJ= \
		sttydefs.o \
		admutil.o \
		tmttydefs.o \
		tmparse.o \
		sttytable.o \
		sttyparse.o 

PRODUCTS = stty ttymon ttyadm sttydefs

all:		$(PRODUCTS)
		@echo "========== make all done!"

stty:		
		$(MAKE) -f stty.mk DFLGS="$(DFLGS)" SHLIBS="$(SHLIBS)" NOSHLIBS="$(NOSHLIBS)"

ttymon:		$(TTYMONOBJ)
		if [ x$(CCSTYPE) = xCOFF ] ; \
		then \
			$(CC) -o ttymon $(TTYMONOBJ) $(LDFLAGS) $(COFFNSL) $(SHLIBS) ; \
		else \
			$(CC) -o ttymon $(TTYMONOBJ) $(LDFLAGS) $(ELFNSL) $(SHLIBS) ; \
		fi

ttyadm:		$(TTYADMOBJ)
		$(CC) -o ttyadm $(TTYADMOBJ) $(LDFLAGS) $(SHLIBS) 

sttydefs:	$(STTYDEFSOBJ)
		if [ x$(CCSTYPE) = xCOFF ] ; \
		then \
			$(CC) -o sttydefs $(STTYDEFSOBJ) $(LDFLAGS) $(COFFNSL) $(SHLIBS) ; \
		else \
			$(CC) -o sttydefs $(STTYDEFSOBJ) $(LDFLAGS) $(ELFNSL) $(SHLIBS) ; \
		fi

lintit:
		$(LINT) $(LINTFLAGS) $(TTYMONSRC)
		$(LINT) $(LINTFLAGS) $(TTYADMSRC)
		$(LINT) $(LINTFLAGS) $(STTYDEFSSRC)

ttymon.o:	ttymon.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/sac.h \
		$(INC)/pwd.h \
		$(INC)/grp.h \
		$(INC)/poll.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h

tmglobal.o:	tmglobal.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/sac.h \
		$(INC)/poll.h \
		$(INC)/stdio.h

tmhandler.o:	tmhandler.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/sac.h \
		$(INC)/poll.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/signal.h \
		$(INC)/termio.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h 

tmpmtab.o:	tmpmtab.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/pwd.h \
		$(INC)/grp.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/signal.h \
		$(INC)/string.h

tmttydefs.o:	tmttydefs.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/string.h \
		$(INC)/termio.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h 

tmparse.o:	tmparse.c \
		$(INC)/stdio.h \
		$(INC)/ctype.h

tmsig.o:	tmsig.c \
		tmextern.h \
		$(INC)/stdio.h \
		$(INC)/signal.h 

tmsac.o:	tmsac.c \
		ttymon.h \
		$(INC)/sac.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/signal.h \
		$(INC)/string.h \
		$(INC)/unistd.h \
		$(INCSYS)/sys/types.h

tmchild.o:	tmchild.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/sac.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/termio.h \
		$(INC)/string.h \
		$(INC)/signal.h \
		$(INC)/poll.h \
		$(INC)/unistd.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h

tmautobaud.o:	tmautobaud.c \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/termio.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h

tmterm.o:	tmterm.c \
		ttymon.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/termio.h \
		$(INC)/string.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h

tmutmp.o:	tmutmp.c \
		$(INC)/sac.h \
		$(INC)/utmp.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/memory.h \
		$(INCSYS)/sys/types.h

tmpeek.o:	tmpeek.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/poll.h \
		$(INC)/stdio.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stropts.h

tmlog.o:	tmlog.c \
		ttymon.h \
		tmstruct.h \
		tmextern.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/string.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/types.h 

tmlock.o:	tmlock.c \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/fcntl.h \
		$(INC)/string.h \
		$(INC)/unistd.h 

tmutil.o:	tmutil.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/string.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h

tmexpress.o:	tmexpress.c \
		ttymon.h \
		tmextern.h \
		tmstruct.h \
		$(INC)/stdio.h \
		$(INC)/stdlib.h \
		$(INC)/unistd.h \
		$(INC)/fcntl.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/string.h 

ulockf.o:	ulockf.c \
		uucp.h \
		parms.h

ttyadm.o:	ttyadm.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/string.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h

admutil.o:	admutil.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/stdio.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/ctype.h \
		$(INC)/string.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h

sttydefs.o:	sttydefs.c \
		ttymon.h \
		tmstruct.h \
		$(INC)/stdio.h \
		$(INC)/unistd.h \
		$(INC)/stdlib.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/termio.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h

install:	all
		$(INS) -o -f $(TTYMONDIR) -m 0544 -u $(ID) -g $(GID) ttymon
		$(INS) -f $(CMDDIR) -m 0755 -u $(ID) -g $(GID) sttydefs
		$(INS) -f $(CMDDIR) -m 0755 -u $(ID) -g $(GID) ttyadm
		$(MAKE) -f stty.mk install
		@echo "========== install done!"

clean:
		-rm -f *.o
	
clobber:	clean
		-rm -f $(PRODUCTS)

FRC:
