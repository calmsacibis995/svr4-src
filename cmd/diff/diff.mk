#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)diff:diff.mk	1.14"
#	diff make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
INC = $(ROOT)/usr/include
RDIR = $(SL)/diff
REL = current
CSID = -r`gsid diff $(REL)`
MKSID = -r`gsid diff.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
INS = install
CFLAGS = -O -I$(INC)
LDFLAGS = -s $(PERFLIBS)
SOURCE = diff.c diffh.c
MAKE = make

VPATH: all ;	ROOT=`pwd`
compile all: diff diffh
	:

diff:
	$(CC) $(CFLAGS) -o diff diff.c $(LDFLAGS)

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin diff 
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin diffh

diffh:
	$(CC) $(CFLAGS) -o diffh diffh.c $(LDFLAGS)

build:	bldmk
	get -p $(CSID) s.diff.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p $(MKSID) s.diff.mk > $(RDIR)/diff.mk

listing:
	pr diff.mk $(SOURCE) | $(LIST)
listmk: ;  pr diff.mk | $(LIST)

edit:
	get -e -p s.diff.src | ntar -g

delta:
	ntar -p $(SOURCE) > diff.src
	delta s.diff.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.diff.mk
mkdelta: ;  delta s.diff.mk

clean:
	:

clobber:
	  rm -f diff diffh

delete:	clobber
	rm -f $(SOURCE)
