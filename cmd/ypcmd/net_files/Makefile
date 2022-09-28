#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ypcmd:net_files/Makefile	1.2.2.1"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
#
#
#	@(#)make.script 1.26 88/10/05 SMI
#
# Set the following variable to "-b" to have yp servers use the domain name
# resolver for hosts not in the current domain. 
#B=-b
B=
SHELL=/sbin/sh
DIR =/etc
DOM = `domainname`
NOPUSH = ""
YPDIR=/usr/sbin
YPDBDIR=/var/yp
YPPUSH=$(YPDIR)/yppush
MAKEDBM=$(YPDIR)/makedbm
ALIASFILE=$(YPDBDIR)/aliases
ALIAS=$(YPDIR)/ypalias

all:  publickey

setup:	
	
publickey.time: $(DIR)/publickey
	-if [ -f $(DIR)/publickey ]; then \
		echo publickey.byname `$(ALIAS) publickey.byname` >> $(ALIASFILE); \
		sort $(ALIASFILE) | uniq > .ypaliases; mv .ypaliases $(ALIASFILE); \
		for i in $(DOM); do \
			sed "/^#/d" < $(DIR)/publickey | \
			$(MAKEDBM) - $(YPDBDIR)/`$(ALIAS) -d $$i`/`$(ALIAS) publickey.byname`; \
		done; \
		touch publickey.time; \
		echo "updated publickey"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) publickey.byname; \
			echo "pushed publickey"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/publickey"; \
	fi

publickey: publickey.time
