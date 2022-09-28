#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/pcnfsd/pcnfsd.mk	1.1"
BINS= pcnfsd
OBJS= pcnfsd.o
SRCS= $(OBJS:.o=.c)
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include/sys
INS = install
INSDIR = $(ROOT)/usr/lib/nfs

DASHO=-O
CPPFLAGS=
CFLAGS= $(DASHO) $(CPPFLAGS) -I$(INC)
LINTFLAGS= -hbax $(CPPFLAGS)
LDFLAGS = -s
LIBS= -lnsl

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

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
