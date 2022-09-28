#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)unpack:unpack.mk	1.12"
#	unpack (pcat) make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd/unpack
RDIR = $(SL)
INS = :
REL = current
CSID = -r`gsid unpack $(REL)`
MKSID = -r`gsid unpack.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INS = install
CFLAGS = -O
LDFLAGS = -s $(PERFLIBS)
SOURCE = unpack.c
MAKE = make

compile all: unpack
	:

unpack:	
	$(CC) $(CFLAGS) -o unpack unpack.c $(LDFLAGS)

pcat:	unpack
	:

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin unpack
	-rm -f $(INSDIR)/pcat
	ln $(INSDIR)/unpack $(INSDIR)/pcat

build:	bldmk
	get -p $(CSID) s.unpack.c $(REWIRE) > $(RDIR)/unpack.c
bldmk:
	get -p $(MKSID) s.unpack.mk > $(RDIR)/unpack.mk
	rm -f $(RDIR)/pcat.mk
	ln $(RDIR)/unpack.mk $(RDIR)/pcat.mk

listing:
	pr unpack.mk $(SOURCE) | $(LIST)
listmk: ;  pr unpack.mk | $(LIST)

edit:
	get -e s.unpack.c

delta:
	delta s.unpack.c

mkedit:  ;  get -e s.unpack.mk
mkdelta: ;  delta s.unpack.mk

clean:
	:

clobber:
	  rm -f unpack

delete:	clobber
	rm -f $(SOURCE)
