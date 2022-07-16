#ident	"@(#)sendmail.mk	1.2	91/09/21	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbsendmail:sendmail.mk	1.1.2.1"


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

#
#	Makefile for sendmail base directory
#
ROOT =
SYMLINK =:
INS = install
ALL=	src/sendmail 
DIRS=	$(ROOT)/usr/ucblib/mqueue
SENDMAIL=$(ROOT)/usr/ucblib/sendmail

all: $(MAKEFILES)
	cd lib; $(MAKE) ${MFLAGS} all
	cd src; $(MAKE) ${MFLAGS} all
	cd aux; $(MAKE) ${MFLAGS} all
	cd cf;  $(MAKE) ${MFLAGS} all
	cd ucblib;  $(MAKE) ${MFLAGS} all

clean:
	cd lib;	$(MAKE) ${MFLAGS} clean
	cd src;	$(MAKE) ${MFLAGS} clean
	cd aux;	$(MAKE) ${MFLAGS} clean
	cd cf;  $(MAKE) ${MFLAGS} clean
	cd ucblib;  $(MAKE) ${MFLAGS} clean
clobber:
	cd lib;	$(MAKE) ${MFLAGS} clobber
	cd src;	$(MAKE) ${MFLAGS} clobber
	cd aux;	$(MAKE) ${MFLAGS} clobber
	cd cf;  $(MAKE) ${MFLAGS} clobber
	cd ucblib;  $(MAKE) ${MFLAGS} clobber

install: all $(ALL) $(DIRS)
	cd src; $(MAKE) ${MFLAGS} install #DESTDIR=${DESTDIR} install
	cd ucblib;  $(MAKE) ${MFLAGS} install
	$(INS) -f $(ROOT)/usr/ucblib -m 444 lib/sendmail.hf 
#	cp  -m 660	/dev/null	$(SENDMAIL).fc
	$(INS) -f $(ROOT)/usr/ucblib -m 444 cf/subsidiary.cf 
	$(INS) -f $(ROOT)/usr/ucblib -m 444 cf/main.cf 
	-ln cf/subsidiary.cf cf/sendmail.cf
	$(INS) -f $(ROOT)/usr/ucblib -m 444 cf/sendmail.cf 
	-rm -f cf/sendmail.cf
	$(INS) -f $(ROOT)/usr/ucblib lib/aliases
	$(INS) -f $(ROOT)/usr/ucb aux/vacation
	$(INS) -f $(ROOT)/usr/ucb aux/mconnect
	$(INS) -f $(ROOT)/usr/ucb aux/mailstats
	-rm -f $(ROOT)/usr/ucb/newaliases
	-rm -f $(ROOT)/usr/ucb/mailq
	$(SYMLINK) $(ROOT)/usr/ucblib/sendmail $(ROOT)/usr/ucb/newaliases
	$(SYMLINK) $(ROOT)/usr/ucblib/sendmail $(ROOT)/usr/ucb/mailq
	-$(CH)$(ROOT)/usr/ucb/newaliases -oA$(ROOT)/usr/ucblib/aliases
	-[ ! -f $(ROOT)/usr/ucblib/mailsurr ] || \
	mv $(ROOT)/usr/ucblib/mailsurr \
			$(ROOT)/usr/ucblib/mailsurr.`/bin/sh -c "echo $$$$"`	
	$(INS) -f $(ROOT)/usr/ucblib cf/mailsurr

$(DIRS):
	-mkdir $(DIRS)
