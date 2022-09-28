#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/automount/automount.mk	1.15.2.1"

#
# Make file for automount
#
BINS= automount

OBJS= nfs_prot.o nfs_server.o nfs_trace.o nfs_cast.o \
	auto_main.o auto_look.o auto_proc.o auto_node.o \
	auto_mount.o auto_all.o \
	mountxdr.o innetgr.o bindresvport.o 
SRCS= $(OBJS:.o=.c)
HDRS= automount.h nfs_prot.h
COFFLIBS= -lnsl_s -lsocket
ELFLIBS = -lnsl -lsocket

INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
INSDIR = $(ROOT)/usr/lib/nfs
CPPFLAGS=
CFLAGS= -O $(CPPFLAGS) -I$(INC)
LDFLAGS = -s
LINTFLAGS= -hbax

#######  AT&T Compatibility
LINT.c= lint $(LINTFLAGS) $(CPPFLAGS)
#######

all: $(BINS)

$(BINS): $(OBJS)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBOPT) $(SHLIBS) $(COFFLIBS) ; \
	else	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBOPT) $(SHLIBS) $(ELFLIBS) ; \
	fi

#nfs_prot.c: nfs_prot.h nfs_prot.x
#	rpcgen -c nfs_prot.x -o $@

#nfs_prot.h: nfs_prot.x
#	rpcgen -h nfs_prot.x -o $@

install: all
	if [ ! -d $(INSDIR) ] ; \
	then mkdir $(INSDIR); \
	fi
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(BINS)

tags: $(SRCS)
	ctags $(SRCS)

lint: $(SRCS)
	$(LINT.c) $(SRCS)

clean:
	-rm -f $(OBJS)

clobber: clean
	-rm -f $(BINS)
