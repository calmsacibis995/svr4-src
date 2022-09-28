#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-streams:log/str.mk	1.5.4.2"

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
SYMLINK = :
USERBIN = $(ROOT)/usr/sbin
CFLAGS = -O -s -I$(INCSYS) -I$(INC) -Di386

PRODUCTS = strace strerr strclean

.c:
	$(CC) $(CFLAGS) -o $* $*.c $(SHLIBS)

all: $(PRODUCTS)

install: all
	-rm -f $(ROOT)/usr/bin/strace
	-rm -f $(ROOT)/usr/bin/strerr
	-rm -f $(ROOT)/usr/bin/strclean
	$(INS) -f $(USERBIN) -m 0100 -u root -g sys strace
	$(INS) -f $(USERBIN) -m 0100 -u root -g sys strerr
	$(INS) -f $(USERBIN) -m 0100 -u root -g sys strclean
	-$(SYMLINK) /usr/sbin/strace $(ROOT)/usr/bin/strace
	-$(SYMLINK) /usr/sbin/strerr $(ROOT)/usr/bin/strerr
	-$(SYMLINK) /usr/sbin/strclean $(ROOT)/usr/bin/strclean
	
clean:
	

clobber: 
	-rm -f $(PRODUCTS)

strace.o:	strace.c \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/time.h \
	$(INCSYS)/sys/stropts.h \
	$(INCSYS)/sys/strlog.h \
	$(FRC)

strerr.o:	strerr.c \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/time.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stropts.h \
	$(INCSYS)/sys/strlog.h \
	$(FRC)

strclean.o:	strclean.c \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/ftw.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stat.h \
	$(INCSYS)/sys/stropts.h \
	$(INCSYS)/sys/strlog.h \
	$(FRC)
