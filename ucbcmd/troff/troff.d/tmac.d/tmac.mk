#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbtroff:troff.d/tmac.d/tmac.mk	1.1.1.1"


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


ROOT=

DIR= $(ROOT)/usr/ucblib

# Files which live in the current directory and are copied to the destination.
#
FILES=	BIG acm.me an bib bl boston boston.toc chars.me deltext.me e eqn.me \
	exp exp.acc exp.eqn exp.ref exp.tbl exp.toc float.me footnote.me \
	index.me local.me m man.macs mmn mmt ms.acc ms.cov ms.eqn ms.ref \
	ms.tbl ms.ths ms.toc null.me refer.me s sh.me sun sunman tbl.me \
	thesis.me tx.map uff v xps 

all:	${FILES}

install:
	if [ ! -d $(DIR)/doctools/tmac ]; then \
		mkdir $(DIR)/doctools/tmac; \
		$(CH)chmod 755 $(DIR)/doctools/tmac; \
		$(CH)chown bin $(DIR)/doctools/tmac; \
		$(CH)chgrp bin $(DIR)/doctools/tmac; \
	fi
	for i in ${FILES}; do \
   	    ($(INS) -f $(DIR)/doctools/tmac -u bin -g bin -m 644 $$i); done
