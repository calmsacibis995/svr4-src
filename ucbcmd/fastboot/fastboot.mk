#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbfastboot:fastboot.mk	1.1.1.1"


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

#	fastboot make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/ucb
RDIR = $(SL)/fastboot
INS = :
SSID = -r`gsid fastboot $(REL)`
MKSID = -r`gsid fastboot.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/ucb
SHSOURCE = fastboot.sh
MAKE = make

compile all: fastboot

fastboot:	fastboot.sh
	cp fastboot.sh fastboot

install:	fastboot
	install -f $(INSDIR) -m 00555 -u bin -g bin fastboot

build: bldmk
	get -p $(SSID) s.fastboot.sh $(REWIRE) > $(RDIR)/fastboot.sh
blkmk: 
	get -p $(MKSID) s.fastboot.mk > $(RDIR)/fastboot.mk

listing:  ;   pr fastboot.mk $(SHSOURCE) $(CSOURCE) | $(LIST)
listfastboot: ;   pr $(SHSOURCE) | $(LIST)
listmk: ;     pr fastboot.mk | $(LIST)

edit:
	get -e s.fastboot.sh
delta:
	delta s.fastboot.sh

mkedit:  ;  get -e s.fastboot.mk
mkdelta: ;  delta s.fastboot.mk

clean:
	:

clobber:
	rm -f fastboot 
delete:	clobber
	rm -f $(SHSOURCE) $(CSOURCE)

