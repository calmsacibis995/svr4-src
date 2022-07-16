#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucbrefer:papers/papers.mk	1.3.3.1"

#     Makefile for refer/papers


ROOT= 
DESTDIR= $(ROOT)/usr/ucblib/reftools
DIR=$(DESTDIR)/papers
PAPERS= Rbstjissue runinv Rv7man

all: $(PAPERS)

$(DIR):
	-mkdir $@
	$(CH)-chmod 755 $@
	$(CH)-chgrp bin $@
	$(CH)-chown bin $@

install: all $(DIR)
	for i in $(PAPERS); do \
		(install -f $(DIR) -m 644 $$i); done
	cd $(DIR); chmod 755 runinv;
