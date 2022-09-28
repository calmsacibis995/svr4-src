#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfsetup:rfsetup.mk	1.9.5.3"


ROOT =
NSLIB = -lns
LIB = $(NSLIB) -lcrypt_i
INSDIR = $(ROOT)/usr/net/servers/rfs
INC = $(ROOT)/usr/include
LOG=-DLOGGING
DEBUG=
LDFLAGS=-s
CFLAGS=-O $(DEBUG) $(LOG) -I$(INC)
INS = install
DIRS = $(ROOT)/usr/net $(ROOT)/usr/net/servers $(ROOT)/usr/net/servers/rfs

all:	rfsetup

rfsetup: rfsetup.o
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) $(CFLAGS) rfsetup.o $(LDFLAGS) $(LIB) -lnsl_s $(SHLIBS) -o rfsetup ; \
	else \
		$(CC) $(CFLAGS) rfsetup.o $(LDFLAGS) $(LIB) -dy -lnsl $(SHLIBS) -o rfsetup ; \
	fi

install: all $(DIRS)
	$(INS) -f $(INSDIR) -m 4550 -u root -g adm rfsetup 

$(DIRS):
	mkdir $@

uninstall:
	(cd $(INSDIR); -rm -f rfsetup )

clean:
	-rm -f *.o

clobber: clean
	-rm -f rfsetup

#### dependencies now follow

rfsetup.o: \
	$(INC)/sys/stropts.h \
	$(INC)/sys/types.h \
	$(INC)/sys/conf.h \
	$(INC)/sys/stat.h \
	$(INC)/sys/rf_cirmgr.h \
	$(INC)/sys/rf_sys.h \
	$(INC)/sys/hetero.h \
	$(INC)/pn.h \
	$(INC)/nserve.h \
	$(INC)/errno.h \
	$(INC)/string.h \
	$(INC)/stdio.h \
	$(INC)/fcntl.h
