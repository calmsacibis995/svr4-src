#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)valtools:valtools.mk	1.1.4.1"
.SUFFIXES:	.df

SRCBASE=./
INC=$(ROOT)/usr/include
#USRLIB=$(ROOT)/usr/lib
DIRS=$(ROOT)/usr/sadm/bin
#LIBADM=$(USRLIB)/libadm.a
#LIBPKG=$(USRLIB)/libpkg.a
#LLIBADM=$(USRLIB)/llib-ladm.ln
#LLIBPKG=$(USRLIB)/llib-lpkg.ln
#LINTLIBS=$(LLIBADM) $(LLIBPKG)
LDFLAGS=-s
CFLAGS=-O
INS=install

.sh:
	cp $< $*
	chmod 755 $*

.c:
	$(CC) $(CFLAGS) -o $@ -I$(INC) $(LDFLAGS) $@.c $(LDLIBPATH) -ladm -lpkg

O_SHFILES=
O_CFILES=ckint ckitem ckpath ckrange ckstr ckyorn ckkeywd puttext \
	ckdate cktime ckuid ckgid
OBJECTS=$(O_CFILES) $(O_SHFILES)

all: $(OBJECTS)
	@:

clean:
	:

clobber: clean
	rm -f $(O_CFILES) $(O_SHFILES) 

strip:
	$(STRIP) $(O_CFILES)

lintit:
	rm -f lint.out
	for file in $(O_CFILES) ;\
	do \
		echo '## lint output for '$$file' ##' >>lint.out ;\
		lint -I $(INC) $$file.c $(LINTLIBS) >>lint.out ;\
	done

$(DIRS):
	mkdir -p $@
	$(CH)chmod 755 $@
	$(CH)chgrp bin $@
	$(CH)chown bin $@

install: all $(DIRS)
	$(INS) -f $(ROOT)/usr/sadm/bin -m 555 -u bin -g bin puttext
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckint
	-rm -f $(ROOT)/usr/sadm/bin/valint
	-rm -f $(ROOT)/usr/sadm/bin/helpint
	-rm -f $(ROOT)/usr/sadm/bin/errint
	ln $(ROOT)/usr/bin/ckint $(ROOT)/usr/sadm/bin/valint
	ln $(ROOT)/usr/bin/ckint $(ROOT)/usr/sadm/bin/helpint
	ln $(ROOT)/usr/bin/ckint $(ROOT)/usr/sadm/bin/errint
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckitem
	-rm -f $(ROOT)/usr/sadm/bin/helpitem
	-rm -f $(ROOT)/usr/sadm/bin/erritem
	ln $(ROOT)/usr/bin/ckitem $(ROOT)/usr/sadm/bin/helpitem
	ln $(ROOT)/usr/bin/ckitem $(ROOT)/usr/sadm/bin/erritem
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckpath
	-rm -f $(ROOT)/usr/sadm/bin/valpath
	-rm -f $(ROOT)/usr/sadm/bin/helppath
	-rm -f $(ROOT)/usr/sadm/bin/errpath
	ln $(ROOT)/usr/bin/ckpath $(ROOT)/usr/sadm/bin/valpath
	ln $(ROOT)/usr/bin/ckpath $(ROOT)/usr/sadm/bin/helppath
	ln $(ROOT)/usr/bin/ckpath $(ROOT)/usr/sadm/bin/errpath
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckrange
	-rm -f $(ROOT)/usr/sadm/bin/valrange
	-rm -f $(ROOT)/usr/sadm/bin/helprange
	-rm -f $(ROOT)/usr/sadm/bin/errange
	ln $(ROOT)/usr/bin/ckrange $(ROOT)/usr/sadm/bin/valrange
	ln $(ROOT)/usr/bin/ckrange $(ROOT)/usr/sadm/bin/helprange
	ln $(ROOT)/usr/bin/ckrange $(ROOT)/usr/sadm/bin/errange
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckstr
	-rm -f $(ROOT)/usr/sadm/bin/valstr
	-rm -f $(ROOT)/usr/sadm/bin/helpstr
	-rm -f $(ROOT)/usr/sadm/bin/errstr
	ln $(ROOT)/usr/bin/ckstr $(ROOT)/usr/sadm/bin/valstr
	ln $(ROOT)/usr/bin/ckstr $(ROOT)/usr/sadm/bin/helpstr
	ln $(ROOT)/usr/bin/ckstr $(ROOT)/usr/sadm/bin/errstr
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckyorn
	-rm -f $(ROOT)/usr/sadm/bin/valyorn
	-rm -f $(ROOT)/usr/sadm/bin/helpyorn
	-rm -f $(ROOT)/usr/sadm/bin/erryorn
	ln $(ROOT)/usr/bin/ckyorn $(ROOT)/usr/sadm/bin/valyorn
	ln $(ROOT)/usr/bin/ckyorn $(ROOT)/usr/sadm/bin/helpyorn
	ln $(ROOT)/usr/bin/ckyorn $(ROOT)/usr/sadm/bin/erryorn
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckkeywd
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin cktime
	-rm -f $(ROOT)/usr/sadm/bin/valtime
	-rm -f $(ROOT)/usr/sadm/bin/helptime
	-rm -f $(ROOT)/usr/sadm/bin/errtime
	ln $(ROOT)/usr/bin/cktime $(ROOT)/usr/sadm/bin/valtime
	ln $(ROOT)/usr/bin/cktime $(ROOT)/usr/sadm/bin/helptime
	ln $(ROOT)/usr/bin/cktime $(ROOT)/usr/sadm/bin/errtime
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckdate
	-rm -f $(ROOT)/usr/sadm/bin/valdate
	-rm -f $(ROOT)/usr/sadm/bin/helpdate
	-rm -f $(ROOT)/usr/sadm/bin/errdate
	ln $(ROOT)/usr/bin/ckdate $(ROOT)/usr/sadm/bin/valdate
	ln $(ROOT)/usr/bin/ckdate $(ROOT)/usr/sadm/bin/helpdate
	ln $(ROOT)/usr/bin/ckdate $(ROOT)/usr/sadm/bin/errdate
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckuid
	-rm -f $(ROOT)/usr/bin/dispuid
	ln $(ROOT)/usr/bin/ckuid $(ROOT)/usr/bin/dispuid
	-rm -f $(ROOT)/usr/sadm/bin/valuid
	-rm -f $(ROOT)/usr/sadm/bin/helpuid
	-rm -f $(ROOT)/usr/sadm/bin/erruid
	-rm -f $(ROOT)/usr/sadm/bin/dispuid
	ln $(ROOT)/usr/bin/ckuid $(ROOT)/usr/sadm/bin/valuid
	ln $(ROOT)/usr/bin/ckuid $(ROOT)/usr/sadm/bin/helpuid
	ln $(ROOT)/usr/bin/ckuid $(ROOT)/usr/sadm/bin/erruid
	ln $(ROOT)/usr/bin/ckuid $(ROOT)/usr/sadm/bin/dispuid
	$(INS) -f $(ROOT)/usr/bin -m 555 -u bin -g bin ckgid
	-rm -f $(ROOT)/usr/bin/dispgid
	ln $(ROOT)/usr/bin/ckgid $(ROOT)/usr/bin/dispgid
	-rm -f $(ROOT)/usr/sadm/bin/valgid
	-rm -f $(ROOT)/usr/sadm/bin/helpgid
	-rm -f $(ROOT)/usr/sadm/bin/errgid
	-rm -f $(ROOT)/usr/sadm/bin/dispgid
	ln $(ROOT)/usr/bin/ckgid $(ROOT)/usr/sadm/bin/valgid
	ln $(ROOT)/usr/bin/ckgid $(ROOT)/usr/sadm/bin/helpgid
	ln $(ROOT)/usr/bin/ckgid $(ROOT)/usr/sadm/bin/errgid
	ln $(ROOT)/usr/bin/ckgid $(ROOT)/usr/sadm/bin/dispgid
