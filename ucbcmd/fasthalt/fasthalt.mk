#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbfasthalt:fasthalt.mk	1.2.1.1"


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#	fasthalt make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/ucb
RDIR = $(SL)/fasthalt
INS = :
SSID = -r`gsid fasthalt $(REL)`
MKSID = -r`gsid fasthalt.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/ucb
SHSOURCE = fasthalt.sh
MAKE = make

compile all: fasthalt

fasthalt:	fasthalt.sh
	cp fasthalt.sh fasthalt

install:	fasthalt
	install -f $(INSDIR) -m 00555 -u bin -g bin fasthalt

build: bldmk
	get -p $(SSID) s.fasthalt.sh $(REWIRE) > $(RDIR)/fasthalt.sh
blkmk: 
	get -p $(MKSID) s.fasthalt.mk > $(RDIR)/fasthalt.mk

listing:  ;   pr fasthalt.mk $(SHSOURCE) $(CSOURCE) | $(LIST)
listfasthalt: ;   pr $(SHSOURCE) | $(LIST)
listmk: ;     pr fasthalt.mk | $(LIST)

edit:
	get -e s.fasthalt.sh
delta:
	delta s.fasthalt.sh

mkedit:  ;  get -e s.fasthalt.mk
mkdelta: ;  delta s.fasthalt.mk

clean:
	:

clobber:
	rm -f fasthalt 
delete:	clobber
	rm -f $(SHSOURCE) $(CSOURCE)

