#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)false:false.mk	1.4.3.1"

ROOT =
DIR = $(ROOT)/usr/bin
SYMLINK = :
INS = install

all:	install

install:
	cp false.sh  false
	$(INS) -f $(DIR) -m 0555 -u bin -g bin false
	-if pdp11 ; then : ; \
	else rm -f $(DIR)/pdp11 ; ln $(DIR)/false $(DIR)/pdp11 ; fi
	-if vax ; then : ; \
	else rm -f $(DIR)/vax ; ln $(DIR)/false $(DIR)/vax ; fi
	-if u370 ; then : ; \
	else rm -f $(DIR)/u370 ; ln $(DIR)/false $(DIR)/u370 ; fi
	-if u3b ; then : ; \
	else rm -f $(DIR)/u3b ; ln $(DIR)/false $(DIR)/u3b ; fi
	-if u3b15 ; then : ; \
	else rm -f $(DIR)/u3b15 ; ln $(DIR)/false $(DIR)/u3b15 ; fi
	-if u3b2 ; then : ; \
	else rm -f $(DIR)/u3b2 ; ln $(DIR)/false $(DIR)/u3b2 ; fi
	-if u3b5 ; then : ; \
	else rm -f $(DIR)/u3b5 ; ln $(DIR)/false $(DIR)/u3b5 ; fi
	-if i286 ; then : ; \
	else rm -f $(DIR)/i286 ; ln $(DIR)/false $(DIR)/i286 ; fi
	-if i386 ; then : ; \
	else rm -f $(DIR)/i386 ; ln $(DIR)/false $(DIR)/i386 ; fi
	-if i486 ; then : ; \
	else rm -f $(DIR)/i486 ; ln $(DIR)/false $(DIR)/i486 ; fi

clean:

clobber:	clean
	-rm false
