#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbtroff:troff.mk	1.2.1.1"


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

#	nroff/troff make file (text subsystem)
#
# DSL 2

CFLAGS = -O
LDFLAGS = -s $(SHLIBS)
INS = :
MAKE = make
INCORE = -DINCORE

compile all:  nroff terms troff fonts tmac

nroff:
	- if u3b2 || u3b5 || u3b15 || i386 ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk nroff INS=$(INS) ROOT=$(ROOT) \
		INCORE= CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
	- if vax || u3b ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk nroff INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi

troff:
	- if u3b2 || u3b5 || u3b15 || i386 ; \
	then cd troff.d;   $(MAKE) -f troff.mk troff INS=$(INS) ROOT=$(ROOT) \
		INCORE= CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
	- if vax || u3b ; \
	then cd troff.d;   $(MAKE) -f troff.mk troff INS=$(INS) ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi

terms:
	cd nroff.d;  $(MAKE) -f nroff.mk terms INS=$(INS) ROOT=$(ROOT) CH=$(CH)

fonts:
	cd troff.d;  $(MAKE) -f troff.mk fonts INS=$(INS) ROOT=$(ROOT) CH=$(CH)

tmac:
	cd troff.d; $(MAKE) -f troff.mk tmac INS=$(INS) ROOT=$(ROOT) CH=$(CH)
install:
	- if u3b2 || u3b5 || u3b15 || i386 ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk install INS=install ROOT=$(ROOT) \
		INCORE= CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
	- if vax || u3b ; \
	then cd nroff.d;   $(MAKE) -f nroff.mk install INS=install ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
	- if u3b2 || u3b5 || u3b15 || i386 ; \
	then cd troff.d;   $(MAKE) -f troff.mk install INS=install ROOT=$(ROOT) \
		INCORE= CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
	- if vax || u3b ; \
	then cd troff.d;   $(MAKE) -f troff.mk install INS=install ROOT=$(ROOT) \
		INCORE=$(INCORE) CH=$(CH) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) ; fi
#	$(MAKE) -f roff.mk INS=install ROOT=$(ROOT) CH= INCORE=$(INCORE) all
#		CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS)
insnroff:
	$(MAKE) -f roff.mk INS=install ROOT=$(ROOT) CH=$(CH) INCORE=$(INCORE) nroff
instroff:
	$(MAKE) -f roff.mk INS=install ROOT=$(ROOT) CH=$(CH) INCORE=$(INCORE) troff

clean:
	cd nroff.d;  $(MAKE) -f nroff.mk clean
	cd troff.d;  $(MAKE) -f troff.mk clean

clobber:
	cd nroff.d;  $(MAKE) -f nroff.mk clobber
	cd troff.d;  $(MAKE) -f troff.mk clobber
