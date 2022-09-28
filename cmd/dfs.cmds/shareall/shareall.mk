#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dfs.cmds:shareall/shareall.mk	1.3.3.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin/
INS = install
FRC =

all: shareall

shareall: shareall.sh 
	cp shareall.sh $(TESTDIR)/shareall
	chmod 555 $(TESTDIR)/shareall

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/shareall

clean:

clobber: clean
	rm -f $(TESTDIR)/shareall
FRC:
