#ident	"@(#)help.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:help.d/help.mk	6.7"
#
#

ROOT = 
CCSLIB = $(ROOT)/usr/ccs/lib
HELPLIB = $(CCSLIB)/help

FILES1 = ad bd cb cm cmds co de default
FILES2 = ge he prs rc un ut vc

all:

install: all
	-[ -d $(HELPLIB) ] || mkdir $(HELPLIB)
	$(CH)-chmod 775 $(HELPLIB)
	-cd $(HELPLIB); rm -f $(FILES2) $(FILES2)
	cp $(FILES1) $(FILES2) $(HELPLIB)
	-cd $(HELPLIB); $(CH)chmod 664 $(FILES1)	$(FILES2)
	-@cd $(HELPLIB); $(CH)chgrp bin $(FILES1) $(FILES2) .
	-@cd $(HELPLIB); $(CH)chown bin $(FILES1) $(FILES2) .

clean:

clobber:
