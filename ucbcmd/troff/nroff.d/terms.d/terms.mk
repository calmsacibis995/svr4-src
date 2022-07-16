#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbtroff:nroff.d/terms.d/terms.mk	1.1.1.1"


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	nroff terminal driving tables make file
#
# DSL 2.

OL = $(ROOT)/
INS = :
INSDIR = ${OL}usr/ucblib/doctools/nterm
FILES = tab.8510 tab.2631 tab.2631-c tab.2631-e tab.300 tab.300-12 tab.300s \
	tab.300s-12 tab.37 tab.382 tab.4000a tab.450 \
	tab.450-12 tab.832 tab.lp tab.tn300 tab.X
IFILES = $(FILES) tab.300S tab.300S-12 tab.4000A

all:	$(FILES)

tab.2631:	a.2631 b.lp
	cat a.2631 b.lp >tab.2631
tab.2631-c:	a.2631-c b.lp
	cat a.2631-c b.lp >tab.2631-c
tab.2631-e:	a.2631-e b.lp
	cat a.2631-e b.lp >tab.2631-e
tab.300:	a.300 b.300
	cat a.300 b.300 >tab.300
tab.300-12:	a.300-12 b.300
	cat a.300-12 b.300 >tab.300-12
tab.300s:	a.300s b.300
	cat a.300s b.300 >tab.300s
tab.300s-12:	a.300s-12 b.300
	cat a.300s-12 b.300 >tab.300s-12
tab.37:	ab.37
	cat ab.37 >tab.37
tab.382:	a.382 b.300
	cat a.382 b.300 >tab.382
tab.4000a:	a.4000a b.300
	cat a.4000a b.300 >tab.4000a
tab.450:	a.450 b.300
	cat a.450 b.300 >tab.450
tab.450-12:	a.450-12 b.300
	cat a.450-12 b.300 >tab.450-12
tab.832:	a.832 b.300
	cat a.832 b.300 >tab.832
tab.8510:	ab.8510
	cat ab.8510 >tab.8510
tab.X:	ab.X
	cat ab.X >tab.X
tab.lp:	a.lp b.lp
	cat a.lp b.lp >tab.lp
tab.tn300:	ab.tn300
	cat ab.tn300 >tab.tn300

install: all
	if [ ! -d $(INSDIR) ]; then rm -f $(INSDIR);  mkdir $(INSDIR); \
		chmod 755 $(INSDIR);  fi
	for i in ${FILES}; do \
   	    ($(INS) -f ${INSDIR} -u bin -g bin -m 644 $$i); done
	rm -f $(INSDIR)/tab.300S $(INSDIR)/tab.300S-12 $(INSDIR)/tab.4000A
	ln $(INSDIR)/tab.300s $(INSDIR)/tab.300S
	ln $(INSDIR)/tab.300s-12 $(INSDIR)/tab.300S-12;
	ln $(INSDIR)/tab.4000a $(INSDIR)/tab.4000A;

clean:

clobber:  clean
	rm -f ${FILES}
