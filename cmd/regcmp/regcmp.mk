#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)regcmp:regcmp.mk	1.8.2.1"
#	regcmp make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd/regcmp
RDIR = $(SL)
INS = install
REL = current
CSID = -r`gsid regcmp $(REL)`
MKSID = -r`gsid regcmp.mk $(REL)`
LIST = lp
CCSBIN=$(ROOT)/usr/ccs/bin
INSDIR = $(CCSBIN)
LINK_MODE=
IFLAG = 
CFLAGS = -O
LDFLAGS = $(IFLAG)
SOURCE = regcmp.c
MAKE = make
STRIP=strip

compile all: regcmp

regcmp:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LINK_MODE) -o regcmp regcmp.c -lgen

install:	all
	cp regcmp regcmp.bak
	$(STRIP) regcmp
	$(INS) -f $(INSDIR) regcmp
	mv regcmp.bak regcmp

build:	bldmk
	get -p $(CSID) s.regcmp.c $(REWIRE) > $(RDIR)/regcmp.c
bldmk:  ;  get -p $(MKSID) s.regcmp.mk > $(RDIR)/regcmp.mk

listing:
	pr regcmp.mk $(SOURCE) | $(LIST)
listmk: ;  pr regcmp.mk | $(LIST)

edit:
	get -e s.regcmp.c

delta:
	delta s.regcmp.c

mkedit:  ;  get -e s.regcmp.mk
mkdelta: ;  delta s.regcmp.mk

clean:
	:

clobber:
	  rm -f regcmp

delete:	clobber
	rm -f $(SOURCE)
