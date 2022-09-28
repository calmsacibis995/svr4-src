#ident	"@(#)cron.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# 	Portions Copyright(c) 1988, Sun Microsystems, Inc.
#	All Rights Reserved.

#ident	"@(#)cron:cron.mk	1.36.1.4"

INS=install

ROOT=
OL=$(ROOT)/
ETC=$(OL)etc
USR=$(OL)usr
VAR=$(OL)var
INSDIR=$(OL)usr/bin
SPL=$(VAR)/spool
SPOOL=$(VAR)/spool/cron
SYMLINK = :
LIB=$(USR)/lib
YACC=yacc
LEX=lex

CRONLIB=$(ETC)/cron.d
CRONSPOOL=$(SPOOL)/crontabs
ATSPOOL=$(SPOOL)/atjobs

XDIRS= $(ROOT) $(ETC) $(USR) $(INSDIR) $(LIB) $(SPL) $(SPOOL)\
      $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

DIRS= $(SPL) $(SPOOL) $(CRONLIB) $(CRONSPOOL) $(ATSPOOL)

CMDS= cron at crontab batch logchecker atq atrm

CFLAGS= -O
LDFLAGS= -s -lcmd
DEFS=

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all:	$(CMDS)

install:	install_cron install_at install_crontab install_batch install_logchecker install_atq install_atrm

install_cron:	cron
	-rm -f $(ROOT)/etc/cron
	$(INS) -f $(ROOT)/usr/sbin -m 500 -u root -g sys cron
	-mkdir ./tmp
	-ln cron.dfl ./tmp/cron
	$(INS) -f $(ROOT)/etc/default -m 444 -u root -g sys ./tmp/cron
	-rm -rf ./tmp
	$(SYMLINK) /usr/sbin/cron $(ROOT)/etc/cron
	$(SYMLINK) /etc/cron.d $(ROOT)/usr/lib/cron

install_at:	at
	$(INS) -f $(INSDIR) -m 4555 -u root -g bin at

install_crontab:	crontab
	$(INS) -f $(INSDIR) -m 4555 -u root -g bin crontab

install_batch:	batch
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin batch

install_logchecker:	logchecker
	-rm -f $(ROOT)/usr/lib/cron/logchecker
	$(INS) -f $(CRONLIB) -m 0544 -u root -g sys logchecker

install_atrm:	atrm
	$(INS) -f $(INSDIR) -m 4555 -u root -g bin atrm

install_atq:	atq
	$(INS) -f $(INSDIR) -m 4555 -u root -g bin atq

libelm.a: elm.o
	$(AR) cr libelm.a elm.o

cron:	cron.o funcs.o libelm.a
	$(CC) $(CFLAGS) cron.o funcs.o libelm.a -o cron $(LDFLAGS) $(SHLIBS)

crontab:	crontab.o permit.o funcs.o
	$(CC) $(CFLAGS) crontab.o permit.o funcs.o -o crontab $(LDFLAGS) $(SHLIBS)

at:	at.o att1.o att2.o funcs.o permit.o
	$(CC) $(CFLAGS) at.o att1.o att2.o funcs.o permit.o -o at $(LDFLAGS) $(SHLIBS)

batch:	batch.sh
	cp batch.sh batch

logchecker:	logchecker.sh
	cp logchecker.sh logchecker

atrm:	atrm.o permit.o funcs.o
	$(CC) $(CFLAGS) atrm.o permit.o funcs.o -o atrm $(LDFLAGS) $(SHLIBS)

atq:	atq.o permit.o funcs.o
	$(CC) $(CFLAGS) atq.o permit.o funcs.o -o atq $(LDFLAGS) $(SHLIBS)

att1.c att1.h:	att1.y
	$(YACC) -d att1.y
	mv y.tab.c att1.c
	mv y.tab.h att1.h

att2.c:	att2.l
	$(LEX) att2.l
	ed - lex.yy.c < att2.ed
	mv lex.yy.c att2.c

att2.o:	att1.h

cron.o:	cron.c cron.h
crontab.o:	crontab.c cron.h
at.o:	at.c cron.h

clean:
	rm -f *.o libelm.a att1.h att1.c att2.c

clobber:	clean
	rm -f $(CMDS)
