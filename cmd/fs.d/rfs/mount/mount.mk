#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfs.cmds:mount/mount.mk	1.15.3.1"



ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc/fs/rfs
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
CFLAGS = -O -s
LDFLAGS = -lns
FRC =
SRCDIR = .

all: mount

mount: mount.c \
	$(INCSYS)/sys/types.h \
	$(INC)/nserve.h \
	$(INC)/netconfig.h \
	$(INCSYS)/sys/stropts.h \
	$(INCSYS)/sys/rf_cirmgr.h \
	$(INCSYS)/sys/vfs.h \
	$(INCSYS)/sys/vnode.h \
	$(INCSYS)/sys/fs/rf_vfs.h \
	$(INCSYS)/sys/rf_sys.h \
	$(INCSYS)/sys/list.h \
	$(INCSYS)/sys/rf_messg.h \
	$(INCSYS)/sys/rf_comm.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INCSYS)/sys/mnttab.h \
	$(INC)/unistd.h \
	$(INC)/errno.h \
	$(INC)/pn.h \
	$(INCSYS)/sys/mount.h \
	$(INCSYS)/sys/conf.h \
	$(FRC)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) -I$(SRCDIR) -I$(INC) -I$(INCSYS) $(CFLAGS) -o $(TESTDIR)/mount mount.c $(LDFLAGS) -lnsl_s $(SHLIBS) ; \
	else \
		$(CC) -I$(SRCDIR) -I$(INC) -I$(INCSYS) $(CFLAGS) -o $(TESTDIR)/mount mount.c $(LDFLAGS) -lnsl $(SHLIBS) ; \
	fi

install: all
	@if [ ! -d $(ROOT)/etc/fs ]; \
		then \
		mkdir $(ROOT)/etc/fs; \
		fi;
	@if [ ! -d $(INSDIR) ]; \
		then \
		mkdir $(INSDIR); \
		fi;
	@if [ ! -d $(ROOT)/usr/lib/fs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs; \
		fi;
	@if [ ! -d $(ROOT)/usr/lib/fs/rfs ]; \
		then \
		mkdir $(ROOT)/usr/lib/fs/rfs; \
		fi;
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/mount
	$(INS) -f $(ROOT)/usr/lib/fs/rfs -m 0555 -u bin -g bin $(TESTDIR)/mount

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/mount

lint:
	lint -I$(SRCDIR) -I$(INC) -I$(INCSYS) mount.c
FRC:
