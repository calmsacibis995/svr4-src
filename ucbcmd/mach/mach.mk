#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbmach:mach.mk	1.2.1.1"


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



#	mach make file

ROOT =
INS = :
OL = $(ROOT)/
SSID = -r`gsid mach $(REL)`
MKSID = -r`gsid mach.mk $(REL)`
LIST = lp
INSDIR = $(OL)usr/ucb
SHSOURCE = mach.sh
MAKE = make

compile all: mach

mach:	mach.sh
	cp mach.sh mach
	$(INS) $(INSDIR) -m 00555 -u bin -g bin mach

install:
	$(MAKE) -f mach.mk INS="install -f" OL=$(OL)

build: bldmk
	get -p $(SSID) s.mach.sh $(REWIRE) > $(RDIR)/mach.sh
blkmk: 
	get -p $(MKSID) s.mach.mk > $(RDIR)/mach.mk

listing:  ;   pr mach.mk $(SHSOURCE) $(CSOURCE) | $(LIST)
listmach: ;   pr $(SHSOURCE) | $(LIST)
listmk: ;     pr mach.mk | $(LIST)

edit:
	get -e s.mach.sh
delta:
	delta s.mach.sh

mkedit:  ;  get -e s.mach.mk
mkdelta: ;  delta s.mach.mk

clean:
	:

clobber:
	rm -f  mach
delete:	clobber
	rm -f $(SHSOURCE) $(CSOURCE)
