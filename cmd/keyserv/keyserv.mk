#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)keyserv:keyserv.mk	1.27.4.1"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
#
RM	= rm -f
LINT	= lint
INC	= $(ROOT)/usr/include
CPPFLAGS= -O -I$(INC) -DYP
CFLAGS	= $(CPPFLAGS)
LDFLAGS=-s
DESTSBIN= $(ROOT)/usr/sbin
DESTBIN = $(ROOT)/usr/bin
LIBRPC	= -lrpcsvc -lnsl
INS	= install

SBINS	= keyserv newkey   
BINS	= keylogout keylogin domainname chkey 
KEYSERV_OBJS = setkey.o detach.o key_generic.o
LIBMPOBJS= pow.o gcd.o msqrt.o mdiv.o mout.o mult.o madd.o util.o
CHANGE_OBJS  = generic.o update.o
OBJS	= $(KEYSERV_OBJS) $(CHANGE_OBJS) $(SBINS:=.o) $(BINS:=.o)
SRCS	= $(OBJS:.o=.c)

all: $(BINS) $(SBINS)

keyserv: $(KEYSERV_OBJS) $(LIBMPOBJS) keyserv.o
	$(CC) $(CFLAGS) -o $@ $(KEYSERV_OBJS) $(LIBMPOBJS) keyserv.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

keylogout: keylogout.o 
	$(CC) $(CFLAGS) -o $@ $@.o $(LIBRPC) $(LDFLAGS) $(LDLIBS)

keylogin: keylogin.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

chkey: $(CHANGE_OBJS) $(LIBMPOBJS) chkey.o
	$(CC) $(CFLAGS) -o $@ $(CHANGE_OBJS) $(LIBMPOBJS) chkey.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

newkey:$(CHANGE_OBJS) $(LIBMPOBJS) newkey.o
	$(CC) $(CFLAGS) -o $@ $(CHANGE_OBJS) $(LIBMPOBJS) newkey.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

generic:$(LIBMPOBJS) generic.o
	$(CC) $(CFLAGS) -o $@ $(LIBMPOBJS) generic.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

update:$(LIBMPOBJS) update.o
	$(CC) $(CFLAGS) -o $@ $(LIBMPOBJS) update.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

domainname: domainname.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LIBRPC) $(LDFLAGS) $(SHLIBS)

lint:
	$(LINT) $(CPPFLAGS) $(SRCS)

clean:
	$(RM) $(OBJS)

clobber: clean
	$(RM) $(SBINS) $(BINS)

install: $(BINS) $(SBINS)
	$(INS) -f $(DESTSBIN) -m 0555 -u root -g sys keyserv
	$(INS) -f $(DESTBIN) -m 0555 -u bin -g bin chkey
	$(INS) -f $(DESTSBIN) -m 0555 -u root -g sys newkey
	$(INS) -f $(DESTBIN) -m 0555 -u bin -g bin domainname
	$(INS) -f $(DESTBIN) -m 0555 -u bin -g bin keylogin
	$(INS) -f $(DESTBIN) -m 0555 -u bin -g bin keylogout
