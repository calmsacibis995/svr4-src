#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  Copyrighted as an unpublished work.
#  (c) Copyright 1990 INTERACTIVE Systems Corporation
#  All rights reserved.
# 
#  RESTRICTED RIGHTS
# 
#  These programs are supplied under a license.  They may be used,
#  disclosed, and/or copied only as permitted under such license
#  agreement.  Any copy must contain the above copyright notice and
#  this restricted rights notice.  Use, copying, and/or disclosure
#  of the programs is strictly prohibited unless otherwise provided
#  in the license agreement.

#ident	"@(#)wd:sys/Makefile	1.1"

DRIVER = wd
FILES = wd.h wdhdw.h

all:	$(FILES)

install: all
	[ -d ../pkg/sys ] || mkdir ../pkg/sys
	for h in $(FILES); \
	do \
	cp $$h ../pkg/sys ; \
	done

clean:
	for h in $(FILES) ; \
	do \
	rm -rf ../pkg/sys/$$h ; \
	done

clobber: clean
