#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)syslogd:syslogd.mk	1.4.1.1"
#
# syslogd.mk:
# makefile for syslogd(1M) daemon
#

INS = install
INC = $(ROOT)/usr/include
SYMLINK = :
OPT = -O
CFLAGS = -I$(INC) ${OPT}
ELFLIBS = -s -lnsl
COFFLIBS = -s -lnsl_s -lc_s
DIR = $(ROOT)/usr/sbin

all:	syslogd

install:	all
		$(INS) -f $(DIR) -m 0100 -u root -g sys syslogd

clean:
	-rm -f *.o

clobber: clean
	-rm -f syslogd

syslogd:	syslogd.c \
		$(INC)/errno.h \
		$(INC)/stdio.h \
		$(INC)/ctype.h \
		$(INC)/signal.h \
		$(INC)/string.h \
		$(INC)/netconfig.h \
		$(INC)/netdir.h \
		$(INC)/tiuser.h \
		$(INC)/utmp.h \
		$(INC)/sys/param.h \
		$(INC)/sys/fcntl.h \
		$(INC)/sys/stropts.h \
		$(INC)/sys/syslog.h \
		$(INC)/sys/strlog.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/time.h \
		$(INC)/sys/utsname.h \
		$(INC)/sys/poll.h \
		$(INC)/sys/wait.h
		if [ x$(CCSTYPE) = xCOFF ] ; \
		then \
			$(CC) $(CFLAGS) -o syslogd syslogd.c $(COFFLIBS) $(SHLIBS) ; \
		else \
			$(CC) $(CFLAGS) -o syslogd syslogd.c $(ELFLIBS) $(SHLIBS) ; \
		fi
