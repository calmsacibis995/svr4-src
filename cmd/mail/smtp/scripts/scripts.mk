#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:smtp/scripts/scripts.mk	1.7.3.1"
# "@(#)scripts.mk	1.9 'attmail mail(1) command'"
# System V Release 3
SVR3USR_SPOOL=	$(ROOT)/usr/spool

# System V Release 4
SVR4USR_SPOOL=	$(ROOT)/var/spool

USR_SPOOL=	$(SVR4USR_SPOOL)
USR_LIB=	/usr/lib
LIBMAIL=	$(USR_LIB)/mail
MAILSURRCMD=	$(LIBMAIL)/surrcmd
RMAILSURRCMD=	$(ROOT)$(LIBMAIL)/surrcmd
INSTALL=	install
LN=		/bin/ln
ETC=		$(ROOT)/etc
ETCRC0=		$(ETC)/rc0.d
ETCRC2=		$(ETC)/rc2.d
ETCRC3=		$(ETC)/rc3.d
ETCINIT=	$(ETC)/init.d
CRONTABS=	$(USR_SPOOL)/cron/crontabs

SCRIPTS=	K74smtpd S88smtpd smtp

all: $(SCRIPTS)

K74smtpd:
	sed -e "s+LIBDIR+$(MAILSURRCMD)+g" < rc0        > $@

S88smtpd:
	sed -e "s+LIBDIR+$(MAILSURRCMD)+g" < rc2        > $@

smtp: cron.scr
	sed -e "s+LIBDIR+$(MAILSURRCMD)+g" < cron.scr   > $@

$(ROOT)$(LIBMAIL):
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown root $@
	$(CH)chgrp mail $@

$(RMAILSURRCMD): $(ROOT)$(LIBMAIL)
	[ -d $@ ] || mkdir $@
	$(CH)chmod 775 $@
	$(CH)chown root $@
	$(CH)chgrp mail $@

install: $(SCRIPTS) $(RMAILSURRCMD)
	$(INSTALL) -f $(CRONTABS) -m 0444 -u root -g sys ./smtp
	$(INSTALL) -f $(ETCRC0)   -m 0444 -u root -g sys ./K74smtpd
	$(INSTALL) -f $(ETCRC2)  -m 0444 -u root -g sys ./S88smtpd
	-$(LN) $(ETCRC2)/S88smtpd $(ETCINIT)/smtpd
	-$(LN) $(ETCRC2)/S88smtpd $(ETCRC3)/S88smtpd

clean:

clobber:
	rm -f $(SCRIPTS)

strip:
