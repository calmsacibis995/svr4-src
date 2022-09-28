#ident	"@(#)oamuser.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamuser:oamuser.mk	1.8.4.1"



DIRS = lib group user

all clean clobber lintit size strip:
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -$(MAKEFFLAGS) $@" ;\
		cd $$i ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd .. ; \
	done

install size strip: all 

install : 
	-[ -d $(ROOT)/usr/sadm ] || mkdir $(ROOT)/usr/sadm
	-[ -d $(ROOT)/etc/skel ] || mkdir $(ROOT)/etc/skel
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) -$(MAKEFFLAGS) $@" ;\
		cd $$i ;\
		$(MAKE) -$(MAKEFLAGS) $(@) ; \
		cd .. ; \
	done
