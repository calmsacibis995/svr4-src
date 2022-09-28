#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oampkg:oampkg.mk	1.5.2.1"

DIRS=\
	libinst pkgadd pkginstall pkgrm pkgremove \
	pkginfo pkgproto pkgchk pkgmk pkgscripts \
	installf pkgtrans

all clobber install clean strip lintit:
	@for i in $(DIRS) ;\
	do \
		echo "\tcd $$i && $(MAKE) $@" ;\
		if cd $$i ;\
		then \
			$(MAKE) $@ ;\
		 	cd .. ;\
		fi ;\
	done

