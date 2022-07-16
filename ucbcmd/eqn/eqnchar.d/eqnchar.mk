#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucbeqn:eqnchar.d/eqnchar.mk	1.2.3.1"

ROOT=

DIR= $(ROOT)/usr/ucblib

# Files which live in the current directory and are copied to the destination.
#
FILES=	eqnchar 

all:	${FILES}

install:
	-mkdir $(DIR)/pub
	$(CH)-chmod 755 $(DIR)/pub
	$(CH)-chgrp bin $(DIR)/pub
	$(CH)-chown bin $(DIR)/pub
	install -f $(DIR)/pub -u bin -g bin -m 644 $(FILES)

clean:
