#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:smtp/src/src.mk	1.16.5.5"
# "@(#)src.mk	1.12 'attmail mail(1) command'"
# To build the SMTP programs:
#
# For a System V Release 3 3B2 running WIN/3B 3.0 TCP/IP:
#	Define CFLAGS=$(SVR3CFLAGS)
#	   and NETLIB=$(SVR3NETLIB)
#	   and USR_SPOOL=$(SVR3USR_SPOOL)
#	   and MAIL=$(SVR3MAIL)
#	   and REALMAIL=$(REALSVR3MAIL)
#	   and REALSMTPQ=$(REALSVR3USR_SPOOL)/smtpq
#
# For a `standard' System V Release 4 (using TLI and netdir):
#	Define CFLAGS=$(SVR4CFLAGS)
#	   and NETLIB=$(SVR4NETLIB)
#	   and USR_SPOOL=$(SVR4USR_SPOOL)
#	   and MAIL=$(SVR4MAIL)
#	   and REALMAIL=$(REALSVR4MAIL)
#	   and REALSMTPQ=$(REALSVR4USR_SPOOL)/smtpq
#

COMMONCFLAGS=	-O

# System V Release 3 Flags
SVR3CFLAGS=	$(COMMONCFLAGS) -DSVR3 -DSOCKET -DBIND -I../..
SVR3NETLIB=	-lnet -lnsl_s
SVR3USR_SPOOL=	$(ROOT)/usr/spool
SVR3MAIL=	$(ROOT)/usr/mail
SVR3LDFLAGS=
REALSVR3MAIL=	/usr/mail
REALSVR3USR_SPOOL=	/usr/spool

# System V Release 4 Flags
SVR4CFLAGS=	$(COMMONCFLAGS) -Xa -DSVR4 -DTLI -DBIND -I../..
SVR4NETLIB=	-Bdynamic -lsocket -lresolv -lnsl -Bstatic -lelf -Bdynamic
SVR4USR_SPOOL=	$(ROOT)/var/spool
SVR4MAIL=	$(ROOT)/var/mail
SVR4LDFLAGS=	
REALSVR4MAIL=	/var/mail
REALSVR4USR_SPOOL=	/var/spool

# Define these by release
CFLAGS=		$(SVR4CFLAGS)
NETLIB=		$(SVR4NETLIB)
USR_SPOOL=	$(SVR4USR_SPOOL)
LDFLAGS=	-s $(SVR4LDFLAGS)
MAIL=		$(SVR4MAIL)
REALMAIL=	$(REALSVR4MAIL)
REALSMTPQ=	$(REALSVR4USR_SPOOL)/smtpq

STRIP=		$(PFX)strip
INSTALL=	install

USR_LIB=	$(ROOT)/usr/lib
USR_LIBMAIL=	$(USR_LIB)/mail
MAILSURRCMD=	$(USR_LIBMAIL)/surrcmd
SMTPQ=		$(USR_SPOOL)/smtpq
REALMAILSURRCMD=	/usr/lib/mail/surrcmd

SMTPLIB=	smtplib.a
SMTPLIBOBJS=	$(SMTPLIB)(aux.o) \
		$(SMTPLIB)(config.o) \
		$(SMTPLIB)(dup2.o) \
		$(SMTPLIB)(from822.o) \
		$(SMTPLIB)(from822ad.o) \
		$(SMTPLIB)(header.o) \
		$(SMTPLIB)(mail.o) \
		$(SMTPLIB)(mx.o) \
		$(SMTPLIB)(netio.o) \
		$(SMTPLIB)(regcomp.o) \
		$(SMTPLIB)(regerror.o) \
		$(SMTPLIB)(regexec.o) \
		$(SMTPLIB)(s5sysname.o) \
		$(SMTPLIB)(smtplog.o) \
		$(SMTPLIB)(to822.o)
LIBMAIL=	$(SMTPLIB) ../../libmail.a

SMTPQER=	smtpqer
SMTPQEROBJ=	smtpqer.o qlib.o

SMTPSCHED=	smtpsched
SMTPSCHEDOBJ=	smtpsched.o qlib.o

FROMSMTP=	fromsmtp
FROMSMTPOBJ=	fromsmtp.o

TOSMTP=		tosmtp
TOSMTPOBJ=	tosmtp.o to822addr.o

SMTP=		smtp
SMTPOBJ=	smtp.o converse.o smtpaux.o to822addr.o

SMTPD=		smtpd
SMTPDOBJ=	smtpd.o conversed.o qlib.o

INSMTPD=	in.smtpd
INSMTPDOBJ=	in.smtpd.o conversed.o

OBJS=		$(FROMSMTP) $(TOSMTP) $(SMTP) $(SMTPD) $(SMTPQER) $(SMTPSCHED)

.c.o: ; $(CC) -c $(CFLAGS) $*.c

all:	$(OBJS)

config.c:
	rm -f config.c
	echo 'char *UPASROOT = "$(REALMAILSURRCMD)/";' >> config.c
	echo 'char *MAILROOT = "$(REALMAIL)/";' >> config.c
	echo 'char *SMTPQROOT = "$(REALSMTPQ)/";' >> config.c

$(SMTPLIB): $(SMTPLIBOBJS)

$(SMTPQER): $(LIBMAIL) $(SMTPQEROBJ)
	$(CC) $(LDFLAGS) -o $(SMTPQER) $(SMTPQEROBJ) $(LIBMAIL) $(NETLIB)

$(SMTPSCHED): $(LIBMAIL) $(SMTPSCHEDOBJ)
	$(CC) $(LDFLAGS) -o $(SMTPSCHED) $(SMTPSCHEDOBJ) $(LIBMAIL) $(NETLIB)

$(FROMSMTP): $(SMTPLIB) $(LIBMAIL) $(FROMSMTPOBJ)
	$(CC) $(LDFLAGS) -o $(FROMSMTP) $(FROMSMTPOBJ) $(SMTPLIB) $(LIBMAIL)

$(TOSMTP): $(SMTPLIB) $(LIBMAIL) $(TOSMTPOBJ)
	$(CC) $(LDFLAGS) -o $(TOSMTP) $(TOSMTPOBJ) $(SMTPLIB) $(LIBMAIL)

$(SMTP): $(LIBMAIL) $(SMTPOBJ)
	$(CC) $(LDFLAGS) -o $(SMTP) $(SMTPOBJ) $(LIBMAIL) $(NETLIB)

$(SMTPD): $(LIBMAIL) $(SMTPDOBJ)
	$(CC) $(LDFLAGS) -o $(SMTPD) $(SMTPDOBJ) $(LIBMAIL) $(NETLIB)

in.smtpd.o: smtpd.c
	$(CC) -c $(CFLAGS) -DSOCKET -DINETD smtpd.c
	mv smtpd.o in.smtpd.o

$(INSMTPD): $(INSMTPDOBJ)
	$(CC) $(LDFLAGS) -o $(INSMTPD) $(INSMTPDOBJ) $(LIBMAIL) $(NETLIB)

$(USR_LIBMAIL):
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown root $@
	$(CH)chgrp mail $@

$(MAILSURRCMD): $(USR_LIBMAIL)
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown root $@
	$(CH)chgrp mail $@

$(MAIL):
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown root $@
	$(CH)chgrp mail $@

$(SMTPQ):
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown uucp $@
	$(CH)chgrp mail $@

install: $(OBJS) $(MAILSURRCMD) $(MAIL) $(SMTPQ)
	$(INSTALL) -f $(MAILSURRCMD) -m 0755 -u bin  -g bin  ./$(FROMSMTP)
	$(INSTALL) -f $(MAILSURRCMD) -m 0755 -u bin  -g bin  ./$(TOSMTP)
	$(INSTALL) -f $(MAILSURRCMD) -m 2755 -u bin  -g mail ./$(SMTP)
	$(INSTALL) -f $(MAILSURRCMD) -m 0755 -u bin  -g bin  ./$(SMTPD)
	$(INSTALL) -f $(MAILSURRCMD) -m 2755 -u bin  -g mail ./$(SMTPQER)
	$(INSTALL) -f $(MAILSURRCMD) -m 6755 -u root -g mail ./$(SMTPSCHED)

clean:
	rm -f *.o core config.c

clobber: clean
	rm -f $(SMTPLIB) $(OBJS)

strip:	$(OBJS)
	$(STRIP) $(OBJS)
