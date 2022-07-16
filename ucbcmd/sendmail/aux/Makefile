#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbsendmail:aux/Makefile	1.6.1.1"


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
#  Makefile for assorted programs related (perhaps distantly) to Sendmail.
#

ALL= mailstats mconnect vacation 	
SRCS= mailstats.c mconnect.c vacation.c

CONVTIME=../src/convtime.o
BZERO=../src/bzero.o
DESTDIR=
INC = $(ROOT)/usr/include

LIB_OPT = -L $(ROOT)/usr/ucblib
LIBDBM=-ldbm
LDFLAGS = -s $(SHLIBS)
O= -O
SYSLIBS= -lnsl -lsocket
COPTS=
CCONFIG= -I../include -I../src -I$(INC) -DSYSV
CFLAGS=	$O $(COPTS) $(CCONFIG)

all: $(ALL)

mailstats: mailstats.o
	$(CC)  $(COPTS) -o $@ $@.o $(SYSLIBS) $(LDFLAGS)

mconnect: mconnect.o
	$(CC)  $(COPTS) -o $@ $@.o -dy $(SYSLIBS) $(LDFLAGS)

vacation: vacation.o
	$(CC)  $(COPTS) -o $@ $@.o $(CONVTIME) $(BZERO) $(SYSLIBS) $(LIB_OPT) $(LIBDBM) $(LDFLAGS)

clobber: 	clean
	rm -f $(ALL)
clean:
	rm -f *.o 

vacation.o: ../include/userdbm.h
