#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbsendmail:src/Makefile	1.4.2.1"


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

ROOT=
INC = $(ROOT)/usr/include
LIB_OPT = -L $(ROOT)/usr/ucblib
LIBDBM=-ldbm
LIBS=  -lnsl -lsocket $(LIB_OPT) $(LIBDBM)
NOYP= getdname.o ypdomain.o noyp.o

OBJS1=	conf.o main.o collect.o parseaddr.o alias.o \
	savemail.o err.o readcf.o stab.o headers.o recipient.o \
	remote.o stats.o usersmtp.o srvrsmtp.o queue.o \
	macro.o util.o clock.o trace.o envelope.o

NOMXOBJS= daemon.o deliver.o
OBJS2=	sysexits.o arpadate.o convtime.o $(NOYP)
OBJS=	$(OBJS1) $(OBJS2)
ALL=	sendmail sendmail.mx

# Resolver for Arpanet-style MX records
MXLIBS= -lresolv -lnsl -lsocket noyp.o $(LIB_OPT) $(LIBDBM)
MXOBJS= daemon-mx.o domain-mx.o deliver-mx.o

O=	-O
COPTS= -DINTER
CCONFIG=-I../include -I$(INC) -DBSD=43 -DSYSV
MXCONFIG=-I../include -I$(INC)/arpa  -DMXDOMAIN $(CCONFIG)
CFLAGS=	$(O) $(COPTS) $(CCONFIG)
LDFLAGS = -s $(SHLIBS)
INS=install

all: $(ALL)

sendmail: $(OBJS1) $(OBJS2) $(NOMXOBJS) version.o getdtblsize.o \
		signal.o bzero.o stricmp.o
	$(CC) $(COPTS) -o sendmail version.o $ getdtblsize.o signal.o \
	bzero.o stricmp.o $(OBJS1) $(OBJS2) $(NOMXOBJS) -dy $(LIBS) $(LDFLAGS)

signal.o:
	$(CC) -I $(ROOT)/usr/ucbinclude -I$(INC) -c $(O) signal.c

daemon-mx.o: daemon.c
	-ln daemon.c daemon-mx.c
	$(CC) -c $O $(MXCONFIG) daemon-mx.c
	rm -f daemon-mx.c

syslog-mx.o: syslog.c
	-ln syslog.c syslog-mx.c
	$(CC) -c $O $(MXCONFIG) syslog-mx.c
	rm -f syslog-mx.c

domain-mx.o: domain.c
	-ln domain.c domain-mx.c
	$(CC) -c $O $(MXCONFIG) domain-mx.c
	rm -f domain-mx.c	

deliver-mx.o: deliver.c
	-ln deliver.c deliver-mx.c
	$(CC) -c $O $(MXCONFIG) deliver-mx.c
	rm -f deliver-mx.c		

sendmail.mx: $(OBJS1) $(OBJS2) $(MXOBJS) version.o bcmp.o bcopy.o index.o \
		getdtblsize.o signal.o bzero.o stricmp.o
	$(CC) $(COPTS) -o sendmail.mx version.o bcmp.o bcopy.o \
	index.o getdtblsize.o signal.o bzero.o stricmp.o $(OBJS1) \
	$(OBJS2) $(MXOBJS) -dy $(MXLIBS) $(LDFLAGS)

install: all
	$(INS) -f $(ROOT)/usr/ucblib -m 4551 -u root sendmail
	$(INS) -f $(ROOT)/usr/ucblib -m 4551 -u root sendmail.mx

$(OBJS1): sendmail.h
$(OBJS): conf.h

sysexits.o: ../include/sysexits.h

#
#  Auxiliary support entries
#

clobber:	clean
	rm -f core sendmail sendmail.mx rmail usersmtp uucp a.out
clean:
	rm -f *.o 

stats.o: mailstats.h
