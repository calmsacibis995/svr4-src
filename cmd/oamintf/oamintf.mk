#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:oamintf.mk	1.10.2.2"

DIR = $(ROOT)/bin
OAMBASE=/usr/sadm/sysadm
BINDIR = $(ROOT)$(OAMBASE)/bin
NADMINDIR = $(ROOT)$(OAMBASE)
MENUDIR = $(ROOT)$(OAMBASE)/menu
INSTALL = install

DOFIRST=libintf

all install clobber clean size strip:
	# \
	# Create our own dependency processing - do the  \
	# things in $(DOFIRST) first. \
	# NOTE: the $(DOFIRST) items get done in *reverse* \
	# order \
	# \
	# First create an 'ed' script to move the DOFIRST \
	# things to the top \
	for f in `ls -d $(DOFIRST)` ; \
	do \
		echo "/$$f/m0" ; \
	done > .ed ;\
	echo "w" >>.ed ;\
	echo "q" >>.ed ;\
	# \
	# create a temp file of all makefiles in this subtree \
	# \
	# Need the set +x so make will not fail due to the error \
	# from find that we are ignoring. \
	find . \( -name Makefile -o -name makefile -o -name "*.mk" \) -print 2>/dev/null | sort | egrep -v "oamintf.mk|OAM4.0|FACE3.2" >.tmp || true ;\
	# \
	# use the 'ed' script to move the DOFIRST things to the top \
	# \
	ed -s .tmp < .ed ;\
	# \
	# now, do the rule \
	# NOTE: could use -ef to pass env. var. down if need to
	for i in `cat .tmp` ;\
	do \
		( \
		echo "cd `dirname $$i` && $(MAKE) -f `basename $$i` $@" ;\
		cd `dirname $$i` && $(MAKE) -f `basename $$i` $@ ;\
		) \
	done ;\
	rm -f .ed .tmp
