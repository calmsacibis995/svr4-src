#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)sa:sa.mk	1.6.1.29"
#/*	sa.mk 1.6.1.22 of 7/16/89	*/
# how to use this makefile
# to make sure all files  are up to date: make -f sa.mk all
#
# to force recompilation of all files: make -f sa.mk all FRC=FRC 
#
# to test new executables before installing in 
# /usr/lib/sa:	make -f sa.mk testbi
#
# to install just one file:	make -f sa.mk safile "INS=/usr/sbin/install"
#
# The sadc and sadp modules must be able to read /dev/kmem,
# which standardly has restricted read permission.
# They must have set-group-ID mode
# and have the same group as /dev/kmem.
# The chmod and chgrp commmands below ensure this.
#
ROOT =
TESTDIR = .
FRC =
INS = install
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lib/sa
CRON= $(ROOT)/var/spool/cron
CRONDIR= $(ROOT)/var/spool/cron/crontabs
CRONTAB= $(ROOT)/var/spool/cron/crontabs/sys
ENTRY1= '0 * * * 0-6 /usr/lib/sa/sa1'
ENTRY2= '20,40 8-17 * * 1-5 /usr/lib/sa/sa1'
ENTRY3= '5 18 * * 1-5 /usr/lib/sa/sa2 -s 8:00 -e 18:01 -i 1200 -A'
MAKE = make
CFLAGS = -O -I$(INC)
FFLAG =
LDFLAGS = -s
ALL = all
SYMLINK = :
 

all:	sadc sar sa1 sa2 perf timex sag sadp 


sadc:: sadc.c sa.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadc sadc.c -lelf $(SHLIBS)
sar:: sar.c sa.h
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sar sar.c -lelf $(SHLIBS)
sa2:: sa2.sh
	cp sa2.sh sa2
sa1:: sa1.sh
	cp sa1.sh sa1
 
perf:: perf.sh
	cp perf.sh perf

timex::	timex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/timex timex.c  $(SHLIBS)
sag::	saga.o sagb.o
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sag saga.o sagb.o  $(SHLIBS)
saga.o:	saga.c saghdr.h
	$(CC) -c $(CFLAGS) saga.c
sagb.o:	sagb.c saghdr.h
	$(CC) -c $(CFLAGS) sagb.c
sadp:: sadp.c 
	$(CC) $(FFLAG) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/sadp sadp.c -lelf $(SHLIBS)

test:		testai

testbi:		#test for before installing
	sh  $(TESTDIR)/runtest new $(ROOT)/usr/src/cmd/sa

testai:		#test for after install
	sh $(TESTDIR)/runtest new

$(INSDIR):
		mkdir $@;
		$(CH)chmod 775 $@;
		$(CH)chown adm $@;
		$(CH)chgrp bin $@;

$(CRON):
		mkdir $@;
		$(CH)chmod 700 $@;
		$(CH)chown root $@;
		$(CH)chgrp sys $@;

$(CRONDIR):
		if [ ! -d $(ROOT)/var/spool ]; \ 
		then mkdir $(ROOT)/var/spool; \
		fi;
		mkdir $@;
		$(CH)chmod 755 $@;
		$(CH)chown root $@;
		$(CH)chgrp sys $@;

$(CRONTAB):
		> $@;
		chmod 644 $@;
		$(CH)chown root $@;
		$(CH)chgrp sys $@;

install: $(ALL) $(INSDIR) $(CRON) $(CRONDIR) $(CRONTAB)
	if [ -f $(CRONTAB) ];\
	then  if grep "sa1" $(CRONTAB) >/dev/null 2>&1 ; then :;\
	else\
		echo $(ENTRY1) >> $(CRONTAB);\
		echo $(ENTRY2) >> $(CRONTAB);\
	fi;\
	if grep "sa2" $(CRONTAB) >/dev/null 2>&1 ; then :;\
	else\
		echo $(ENTRY3) >> $(CRONTAB);\
	fi;\
	fi;
	-rm -f $(ROOT)/usr/bin/sar
	-rm -f $(ROOT)/usr/bin/sag
	-rm -f $(ROOT)/usr/bin/sadp
	-rm -f $(ROOT)/usr/bin/sar
	-rm -f $(ROOT)/etc/rc2.d/S21perf
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u bin -g bin $(TESTDIR)/sar 
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/sa2
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/sa1
	$(INS) -f $(ROOT)/etc/init.d -u root -g sys -m 444 $(TESTDIR)/perf
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u bin -g bin $(TESTDIR)/sag
	$(INS) -f $(ROOT)/usr/bin -m 0555 -u bin -g sys $(TESTDIR)/timex
	$(INS) -f $(ROOT)/usr/sbin -m 2555 -u bin -g sys $(TESTDIR)/sadp 
	$(INS) -o -f $(INSDIR) -m 2555 -u bin -g sys $(TESTDIR)/sadc
	ln $(ROOT)/etc/init.d/perf $(ROOT)/etc/rc2.d/S21perf
	$(SYMLINK) /usr/sbin/sar $(ROOT)/usr/bin/sar
	$(SYMLINK) /usr/sbin/sadp $(ROOT)/usr/bin/sadp
	$(SYMLINK) /usr/sbin/sag $(ROOT)/usr/bin/sag

clean:
	-rm -f *.o

clobber:	clean
		-rm -f sadc sar sa1 sa2 perf sag timex sadp 

FRC:
