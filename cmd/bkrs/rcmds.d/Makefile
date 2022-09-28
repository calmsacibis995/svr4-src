#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/Makefile	1.1.2.1"

INC = $(ROOT)/usr/include
OAMBASE=$(ROOT)/usr/sadm/sysadm
SBIN=$(OAMBASE)/bin
INC = $(ROOT)/usr/include
CFLAGS = -O -I.. -I$(INC) -s
INSTALL = install

SHELLS = add_cron filechk filemsg owrchk runbackf runbacku runstore runstore2
CFILES = Ckdev chg_cron del_cron display format getchg getdel prtask setquote ts_valid slash

all:	$(CFILES) $(SHELLS)

$(CFILES):
	$(CC) $(CFLAGS) $(@).c -o $(@)

$(SHELLS):
	-cp $(@).sh $(@)

install:	all
	-mkdir -p $(SBIN)

	-for i in $(CFILES); \
	do \
		$(INSTALL) -f $(SBIN) -m 0755 -u bin -g bin $$i ;\
	done

	-for i in $(SHELLS); \
	do \
		$(INSTALL) -f $(SBIN) -m 0755 -u bin -g bin $$i ;\
	done

clean:
	-rm -f *.o $(SHELLS) $(CFILES)

clobber: clean
	@echo "No targets to clobber!"

