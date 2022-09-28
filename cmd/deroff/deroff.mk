#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)deroff:deroff.mk	1.11"

ROOT = 
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd/deroff
RDIR = $(SL)
INS = install
REL = current
CSID = -r`gsid deroff $(REL)`
MKSID = -r`gsid deroff.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
B20 =
CFLAGS = -O $(B20)
LDFLAGS = -s $(PERFLIBS)
SOURCE = deroff.c
MAKE = make

compile all: deroff
	:

deroff:
	$(CC) $(CFLAGS) $(LDFLAGS) -o deroff deroff.c

install:	all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin deroff

build:	bldmk
	get -p $(CSID) s.deroff.c $(REWIRE) > $(RDIR)/deroff.c
bldmk:  ;  get -p $(MKSID) s.deroff.mk > $(RDIR)/deroff.mk

listing:
	pr deroff.mk $(SOURCE) | $(LIST)
listmk: ;  pr deroff.mk | $(LIST)

edit:
	get -e s.deroff.c

delta:
	delta s.deroff.c

mkedit:  ;  get -e s.deroff.mk
mkdelta: ;  delta s.deroff.mk

clean:
	:

clobber:  clean
	  rm -f deroff

delete:	clobber
	rm -f $(SOURCE)
