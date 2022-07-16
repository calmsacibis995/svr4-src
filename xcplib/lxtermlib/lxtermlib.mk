#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.	  

#ident	"@(#)xcplxterm:lxtermlib.mk	1.1"

#
#	@(#) lxtermlib.mk 1.1 90/04/06 lxtermlib:lxtermlib.mk
#
LGTXTNM=_LIBC_TEXT

CFLAGS	= -O -DCM_N -DCM_GT -DCM_B -DCM_D
LIBTERM	= libxtermlib.a
INS=install
LIBOBJS	= \
	$(LIBTERM)(termcap.o) \
	$(LIBTERM)(tgoto.o) \
	$(LIBTERM)(tputs.o)

.PRECIOUS:	$(LIBTERM)

# standard targets
all:	$(LIBOBJS)


install:		all
	$(INS) -f $(ROOT)/usr/lib -u root -g sys -m 644 $(LIBTERM)
	-ln $(ROOT)/usr/lib/libxtermlib.a $(ROOT)/usr/lib/libxtermcap.a

clean:
	rm -f $(LIBTERM)

clobber: clean
