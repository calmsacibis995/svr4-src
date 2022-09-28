#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)true:true.mk	1.4.4.1"

ROOT =
DIR = $(ROOT)/usr/bin
INS = install

all:	install

install:
	cp true.sh  true
	$(INS) -f $(DIR) -m 0555 -u bin -g bin true
	-if pdp11 ; then rm -f $(DIR)/pdp11 ; ln $(DIR)/true $(DIR)/pdp11 ; fi
	-if vax ; then rm -f $(DIR)/vax ; ln $(DIR)/true $(DIR)/vax ; fi
	-if u370 ; then rm -f $(DIR)/u370 ; ln $(DIR)/true $(DIR)/u370 ; fi
	-if u3b ; then rm -f $(DIR)/u3b ; ln $(DIR)/true $(DIR)/u3b ; fi
	-if u3b15 ; then rm -f $(DIR)/u3b15 ; ln $(DIR)/true $(DIR)/u3b15 ; fi
	-if u3b2 ; then rm -f $(DIR)/u3b2 ; ln $(DIR)/true $(DIR)/u3b2 ; fi
	-if u3b5 ; then rm -f $(DIR)/u3b5 ; ln $(DIR)/true $(DIR)/u3b5 ; fi
	-if i286 ; then rm -f $(DIR)/i286 ; ln $(DIR)/true $(DIR)/i286 ; fi
	-if i386 ; then rm -f $(DIR)/i386 ; ln $(DIR)/true $(DIR)/i386 ; fi
	-if i486 ; then rm -f $(DIR)/i486 ; ln $(DIR)/true $(DIR)/i486 ; fi

clean:

clobber:	clean
	-rm true
