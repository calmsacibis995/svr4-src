#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devmgmt:data/data.mk	1.3.5.1"



FILE=device.tab dgroup.tab
INSTALL=install
OBJECTS=$(FILE)
PROTO=../Prototype

all		: $(FILE) 

install: all
	@eval `sed -e '/^![^=]*=/!d' -e 's/^!//' $(PROTO)` ;\
	mkpath() { \
		while true ;\
		do \
			tmpdir=$$1 ;\
			[ -d $$tmpdir ] && break ;\
			while [ ! -d $$tmpdir ] ;\
			do \
				lastdir=$$tmpdir ;\
				tmpdir=`dirname $$tmpdir` ;\
			done ;\
			mkdir $$lastdir ;\
		done ;\
	} ;\
	for object in $(OBJECTS) ;\
	do \
		if entry=`grep "[ 	/]$$object[= 	]" $(PROTO)` ;\
		then \
			set -- $$entry ;\
			path=`eval echo $$3` ;\
			expr $$path : '[^/]' >/dev/null && \
				path=$(BASEDIR)/$$path ;\
			dir=$(ROOT)`dirname $$path` ;\
			[ ! -d $$dir ] && mkpath $$dir ;\
			$(INSTALL) -f $$dir -m $$4 -u $$5 -g $$6 $$object ;\
		else \
			echo "unable to install $$object" ;\
		fi ;\
	done
.DEFAULT	:
