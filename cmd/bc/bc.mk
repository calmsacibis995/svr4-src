#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)bc:bc.mk	1.12"
#	bc make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
RDIR = $(SL)/bc
INS = install
INC = $(ROOT)/usr/include
REL = current
CSID = -r`gsid bc $(REL)`
MKSID = -r`gsid bc.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
B20 = `if u370; then echo "-b2,0"; fi`
CFLAGS = -O $(B20) $(FFLAG) -I$(INC)
LDFLAG = -s
SOURCE = bc.y lib.b.data
FILES = bc.c
MAKE = make
YACC = yacc

compile all: bc lib.b
	:

bc:	$(FILES)
	$(CC) $(CFLAGS) $(LDFLAG) -o bc $(FILES) $(SHLIBS)

$(FILES):
	-$(YACC) bc.y && mv y.tab.c bc.x
	cp bc.x bc.c

lib.b:
	cp lib.b.data lib.b

install:	all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin bc
	$(INS) -f $(INSLIB) -m 0444 -u bin -g bin lib.b

build:	bldmk
	get -p $(CSID) s.bc.src $(REWIRE) | ntar -d $(RDIR) -g
	cd $(RDIR); $(YACC) bc.y; mv y.tab.c bc.x

bldmk:  ;  get -p $(MKSID) s.bc.mk > $(RDIR)/bc.mk

listing:
	pr bc.mk $(SOURCE) | $(LIST)
listmk: ;  pr bc.mk | $(LIST)

edit:
	get -e -p s.bc.src | ntar -g

delta:
	ntar -p $(SOURCE) > bc.src
	delta s.bc.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.bc.mk
mkdelta: ;  delta s.bc.mk

clean:
	:

clobber:	clean
	rm -f bc bc.c lib.b bc.x

delete:	clobber
	rm -f $(SOURCE) bc.x
