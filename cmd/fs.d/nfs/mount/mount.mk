#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/mount/mount.mk	1.18.2.1"

BINS= mount
OBJS= mount.o mountxdr.o bindresvport.o 
SRCS= $(OBJS:.o=.c)
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
INSDIR = $(ROOT)/usr/lib/fs/nfs
SYMLINK = :

CPPFLAGS=
CFLAGS= $(CPPFLAGS) -I$(INC)
LINTFLAGS= -hbax $(CPPFLAGS)
LDFLAGS = -s
COFFLIBS= -lnsl_s -lsocket
ELFLIBS = -lnsl -lsocket 

all: $(BINS)

$(BINS): $(OBJS)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBOPT) $(SHLIBS) $(COFFLIBS) ; \
	else	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBOPT) $(SHLIBS) $(ELFLIBS) ; \
	fi

install: all
	if [ ! -d $(INSDIR) ] ; \
	then mkdir -p $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(BINS)

lint:
	lint $(LINTFLAGS) $(SRCS)

tags: $(SRCS)
	ctags $(SRCS)

clean:
	-rm -f $(OBJS)

clobber: clean
	-rm -f $(BINS)
