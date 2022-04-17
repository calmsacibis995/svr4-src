#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)acct:acct.mk	1.9.4.2"
ROOT =
TESTDIR = .
FRC =
INS = install
MAKE = make
ARGS =
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/acct
CONFIGDIR = $(ROOT)/etc/acct
ADMDIR = $(ROOT)/var/adm
USRBIN = $(ROOT)/usr/bin
ETCINIT = $(ROOT)/etc/init.d
WKDIR	= $(ADMDIR) $(ADMDIR)/acct $(ADMDIR)/acct/nite $(ADMDIR)/acct/fiscal $(ADMDIR)/acct/sum
CC = cc
CFLAGS = -O -I$(INC)
OPTFLAGS =
LFLAGS = -s
LIB = lib/a.a
LDFLAGS = -lgen

all:	library acctcms acctcom acctcon acctcon1\
	acctcon2 acctdisk acctdusg acctmerg accton\
	acctprc acctprc1 acctprc2 acctwtmp\
	closewtmp diskusg fwtmp wtmpfix utmp2wtmp\
	acct chargefee ckpacct dodisk lastlogin\
	monacct nulladm prctmp prdaily\
	prtacct remove runacct\
	shutacct startup turnacct holtable \
	awkecms awkelus

library:
		cd lib; $(MAKE) "INS=$(INS)" "CC=$(CC)" "OPTFLAGS=$(OPTFLAGS)" "CFLAGS=$(CFLAGS)"

acctcms:	$(LIB) acctcms.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctcms.c $(LIB) -o $(TESTDIR)/acctcms $(SHLIBS)

acctcom:	$(LIB) acctcom.c $(FRC)
		$(CC)  $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctcom.c $(LIB) -o $(TESTDIR)/acctcom $(LDFLAGS) $(SHLIBS)

acctcon:	$(LIB) acctcon.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon.c $(LIB) -o $(TESTDIR)/acctcon $(SHLIBS)

acctcon1:	$(LIB) acctcon1.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon1.c $(LIB) -o $(TESTDIR)/acctcon1 $(SHLIBS)

acctcon2:	acctcon2.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctcon2.c -o $(TESTDIR)/acctcon2 $(SHLIBS)

acctdisk:	acctdisk.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctdisk.c -o $(TESTDIR)/acctdisk $(SHLIBS)

acctdusg:	acctdusg.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctdusg.c -o $(TESTDIR)/acctdusg $(SHLIBS)

acctmerg:	$(LIB) acctmerg.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctmerg.c $(LIB) -o $(TESTDIR)/acctmerg $(SHLIBS)

accton:		accton.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			accton.c -o $(TESTDIR)/accton $(SHLIBS)

acctprc1:	$(LIB) acctprc1.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc1.c $(LIB) -o $(TESTDIR)/acctprc1 $(SHLIBS)

acctprc:	$(LIB) acctprc.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc.c $(LIB) -o $(TESTDIR)/acctprc $(SHLIBS)

acctprc2:	acctprc2.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctprc2.c -o $(TESTDIR)/acctprc2 $(SHLIBS)

acctwtmp:	acctwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			acctwtmp.c -o $(TESTDIR)/acctwtmp $(SHLIBS)

closewtmp:	closewtmp.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			closewtmp.c -o $(TESTDIR)/closewtmp $(SHLIBS)

diskusg:	diskusg.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			diskusg.c -o $(TESTDIR)/diskusg $(SHLIBS)

fwtmp:		fwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			fwtmp.c -o $(TESTDIR)/fwtmp $(SHLIBS)

wtmpfix:	wtmpfix.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			wtmpfix.c -o $(TESTDIR)/wtmpfix $(SHLIBS)

utmp2wtmp:	utmp2wtmp.c $(FRC)
		$(CC) $(CFLAGS) $(OPTFLAGS) $(LFLAGS) $(FFLAG) \
			utmp2wtmp.c -o $(TESTDIR)/utmp2wtmp $(SHLIBS)

acct:		acct.sh $(FRC)
		cp acct.sh $(TESTDIR)/acct

chargefee:	chargefee.sh $(FRC)
		cp chargefee.sh $(TESTDIR)/chargefee

ckpacct:	ckpacct.sh $(FRC)
		cp ckpacct.sh $(TESTDIR)/ckpacct

dodisk:		dodisk.sh $(FRC)
		cp dodisk.sh $(TESTDIR)/dodisk

monacct:	monacct.sh $(FRC)
		cp monacct.sh $(TESTDIR)/monacct

lastlogin:	lastlogin.sh $(FRC)
		cp lastlogin.sh $(TESTDIR)/lastlogin

nulladm:	nulladm.sh $(FRC)
		cp nulladm.sh $(TESTDIR)/nulladm

prctmp:		prctmp.sh $(FRC)
		cp prctmp.sh $(TESTDIR)/prctmp

prdaily:	prdaily.sh $(FRC)
		cp prdaily.sh $(TESTDIR)/prdaily

prtacct:	prtacct.sh $(FRC)
		cp prtacct.sh $(TESTDIR)/prtacct

remove:		remove.sh $(FRC)
		cp remove.sh $(TESTDIR)/remove

runacct:	runacct.sh $(FRC)
		cp runacct.sh $(TESTDIR)/runacct

shutacct:	shutacct.sh $(FRC)
		cp shutacct.sh $(TESTDIR)/shutacct

startup:	startup.sh $(FRC)
		cp startup.sh $(TESTDIR)/startup

turnacct:	turnacct.sh $(FRC)
		cp turnacct.sh $(TESTDIR)/turnacct

holtable:	holidays $(FRC)

awkecms:	ptecms.awk $(FRC)

awkelus:	ptelus.awk $(FRC)

$(INSDIR):
		mkdir $@;
		chmod 775 $@;
		$(CH) chown bin $@
		$(CH) chgrp bin $@

$(WKDIR):
		mkdir $@;
		chmod 775 $@;
		$(CH) chown adm $@
		$(CH) chgrp adm $@

$(CONFIGDIR):
		mkdir $@;
		chmod 775 $@;
		$(CH) chown adm $@
		$(CH) chgrp adm $@

install:	all $(INSDIR) $(WKDIR) $(CONFIGDIR)
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcms
		$(INS) -f $(USRBIN) $(TESTDIR)/acctcom
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctcon2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdisk
		$(INS) -f $(INSDIR) $(TESTDIR)/acctdusg
		$(INS) -f $(INSDIR) $(TESTDIR)/acctmerg
		$(INS) -f $(INSDIR) -u root -g adm -m 4755 $(TESTDIR)/accton
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc1
		$(INS) -f $(INSDIR) $(TESTDIR)/acctprc2
		$(INS) -f $(INSDIR) $(TESTDIR)/acctwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/closewtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/fwtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/diskusg
		$(INS) -f $(INSDIR) $(TESTDIR)/utmp2wtmp
		$(INS) -f $(INSDIR) $(TESTDIR)/wtmpfix
		$(INS) -f $(ETCINIT) -u root -g sys -m 444 $(TESTDIR)/acct
		$(INS) -f $(INSDIR) $(TESTDIR)/chargefee
		$(INS) -f $(INSDIR) $(TESTDIR)/ckpacct
		$(INS) -f $(INSDIR) $(TESTDIR)/dodisk
		$(INS) -f $(INSDIR) $(TESTDIR)/monacct
		$(INS) -f $(INSDIR) $(TESTDIR)/lastlogin
		$(INS) -f $(INSDIR) $(TESTDIR)/nulladm
		$(INS) -f $(INSDIR) $(TESTDIR)/prctmp
		$(INS) -f $(INSDIR) $(TESTDIR)/prdaily
		$(INS) -f $(INSDIR) $(TESTDIR)/prtacct
		$(INS) -f $(INSDIR) $(TESTDIR)/remove
		$(INS) -f $(INSDIR) $(TESTDIR)/runacct
		$(INS) -f $(INSDIR) $(TESTDIR)/shutacct
		$(INS) -f $(INSDIR) $(TESTDIR)/startup
		$(INS) -f $(INSDIR) $(TESTDIR)/turnacct
		$(INS) -f $(CONFIGDIR) -m 664 $(TESTDIR)/holidays
		$(INS) -f $(INSDIR) $(TESTDIR)/ptecms.awk
		$(INS) -f $(INSDIR) $(TESTDIR)/ptelus.awk


clean:
		-rm -f *.o
		cd lib; $(MAKE) clean

clobber:	clean
		-rm -f					\
			$(TESTDIR)/acctcms		\
			$(TESTDIR)/acctcom		\
			$(TESTDIR)/acctcon		\
			$(TESTDIR)/acctcon1		\
			$(TESTDIR)/acctcon2		\
			$(TESTDIR)/acctdisk		\
			$(TESTDIR)/diskusg		\
			$(TESTDIR)/acctdusg		\
			$(TESTDIR)/acctmerg		\
			$(TESTDIR)/accton		\
			$(TESTDIR)/acctprc		\
			$(TESTDIR)/acctprc1		\
			$(TESTDIR)/acctprc2		\
			$(TESTDIR)/acctwtmp		\
			$(TESTDIR)/closewtmp		\
			$(TESTDIR)/fwtmp		\
			$(TESTDIR)/utmp2wtmp		\
			$(TESTDIR)/wtmpfix
		-rm -f					\
			$(TESTDIR)/acct			\
			$(TESTDIR)/chargefee		\
			$(TESTDIR)/ckpacct		\
			$(TESTDIR)/dodisk		\
			$(TESTDIR)/lastlogin		\
			$(TESTDIR)/nulladm		\
			$(TESTDIR)/monacct		\
			$(TESTDIR)/prctmp		\
			$(TESTDIR)/prdaily		\
			$(TESTDIR)/prtacct		\
			$(TESTDIR)/remove		\
			$(TESTDIR)/runacct		\
			$(TESTDIR)/shutacct		\
			$(TESTDIR)/startup		\
			$(TESTDIR)/turnacct
		cd lib; $(MAKE) clobber

FRC:
