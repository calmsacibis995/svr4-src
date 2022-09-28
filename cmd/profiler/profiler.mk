#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)profil-3b15:profiler.mk	1.4.3.1"
ROOT =
INC = $(ROOT)/usr/include
MAKE = make "CC=$(CC)"
CFLAGS = -I$(INC) -Uvax -Uu3b15 -Updp11 -Di386 -O
LDFLAGS = -s
LIBELF = -lelf
INSDIR = $(ROOT)/usr/sbin
SYMLINK = :
INS = install

all:	prfld prfdc prfpr prfsnap prfstat

install:	all
	-rm -f $(ROOT)/etc/prfld
	-rm -f $(ROOT)/etc/prfdc
	-rm -f $(ROOT)/etc/prfpr
	-rm -f $(ROOT)/etc/prfsnap
	-rm -f $(ROOT)/etc/prfstat
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin prfld
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin prfdc
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin prfpr
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin prfsnap
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin prfstat
	-$(SYMLINK) /usr/sbin/prfld $(ROOT)/etc/prfld
	-$(SYMLINK) /usr/sbin/prfdc $(ROOT)/etc/prfdc
	-$(SYMLINK) /usr/sbin/prfpr $(ROOT)/etc/prfpr
	-$(SYMLINK) /usr/sbin/prfsnap $(ROOT)/etc/prfsnap
	-$(SYMLINK) /usr/sbin/prfstat $(ROOT)/etc/prfstat


prfld:		prfld.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfld prfld.c $(LIBELF) $(SHLIBS)

prfdc:		prfdc.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfdc prfdc.c $(SHLIBS)

prfpr:		prfpr.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfpr prfpr.c $(LIBELF) $(SHLIBS)

prfsnap:	prfsnap.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfsnap prfsnap.c $(SHLIBS)

prfstat:	prfstat.c
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o prfstat prfstat.c $(SHLIBS)

clean:
	-rm -f prfdc prfld prfpr prfsnap prfstat

clobber:	clean
