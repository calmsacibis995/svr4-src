#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oawk:oawk.mk	1.8"
ROOT=
OL = /
SL = $(ROOT)/usr/src/cmd
INSDIR = $(ROOT)/usr/bin
RDIR = $(SL)/oawk
YACCRM=-rm
TESTDIR = .
FRC =
INS = install
LN = :
B10 =
B11 =
IFLAG = -i
CFLAGS=-O $(FFLAG)
YFLAGS=-d
LDFLAGS = -s $(IFLAG)
REL = current
LIST = lp
FILES=awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o freeze.o
SOURCE=EXPLAIN README awk.def awk.g.y awk.h awk.lx.l b.c lib.c main.c parse.c \
	freeze.c run.c tokenscript tran.c

all:  oawk

oawk:	$(FILES) awk.g.o
	$(CC) $(CFLAGS) $(LDFLAGS) awk.g.o  $(FILES)  -lm -o  $(TESTDIR)/oawk

y.tab.h:	awk.g.o

awk.h:	y.tab.h
	-cp y.tab.h awk.h

$(FILES):	awk.h awk.def $(FRC)

token.c:	awk.h tokenscript
	$(CH)chmod u+w token.c
	ed - <tokenscript

src:	$(SOURCE) test.a tokenscript makefile README
	cp $? $(RDIR)
	touch src

get:
	for i in $(SOURCE) oawk.mk tokenscript README; do cp $(RDIR)/$$i .; done

bin:
	cp a.out $(INSDIR)/oawk
	strip $(INSDIR)/oawk

profile:	awk.g.o $(FILES) mon.o
	$(CC) -p -i awk.g.o $(FILES) mon.o -lm

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr WISH $(SOURCE) oawk.mk tokenscript README EXPLAIN

lint:
	lint -pu b.c main.c token.c tran.c run.c lib.c parse.c -lm |\
		egrep -v '^(error|free|malloc)'

proctab.c:	./makeprctab
	./makeprctab >proctab.c

#Compile makeprctab.c and token.c in the native environment. This is
#necessary to ensure that AWK will compile under a cross-compilation
#environment.

./makeprctab:	awk.h
	cc -O -c makeprctab.c
	cc -O -c token.c
	cc -o ./makeprctab makeprctab.o token.o
	$(CC) -O -c token.c

clean:
	rm -f a.out *.o t.* *temp* *.out *junk* y.tab.* awk.h ./makeprctab proctab.c

diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i $(RDIR) | ind; done
lcomp:
	-rm -f  [b-z]*.o
	lcomp b.c main.c token.c tran.c run.c lib.c parse.c freeze.c proctab.c   *.o -lm

FRC:

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/oawk
	-ln $(INSDIR)/oawk $(INSDIR)/awk

build:	bldmk
	get -p -r`gsid oawk $(REL)` s.awk.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR) ; $(YACC) $(YFLAGS) awk.g.y
	cd $(RDIR) ; mv y.tab.c  awk.g.c ; rm -f y.tab.h

bldmk:
	get -p -r`gsid oawk.km $(REL) s.oawk.mk >$(RDIR)/oawk.mk

listing:
	pr oawk.mk $(SOURCE) | $(LIST)

clobber:	clean
	-rm  -f  $(TESTDIR)/oawk

delete:	clobber
	rm -f $(SOURCE)

