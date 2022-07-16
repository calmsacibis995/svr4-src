#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbplot:libplot/libplot.mk	1.1.3.1"

#	Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
#	All Rights Reserved.

#       Makefile for libplot


ALL=	libf77plot libplot lib300 lib300s lib4013 lib4014 lib450 libvt0 \
	libplotaed libplotbg libplotdumb libplotgigi libplot2648 \
	libplot7221 libplotimagen
SUBDIRS=tf77 plot t4013 t4014 t300 t300s t450 vt0\
	aed bitgraph dumb gigi hp2648 hp7221 imagen

all:	${ALL}

libf77plot: FRC
	cd tf77; make -f tf77.mk ${MFLAGS}

libplot: FRC
	cd plot; make -f plot.mk ${MFLAGS}

lib4013: FRC
	cd t4013; make -f t4013.mk ${MFLAGS}

lib4014: FRC
	cd t4014; make -f t4014.mk ${MFLAGS}

lib300: FRC
	cd t300; make -f t300.mk ${MFLAGS}

lib300s: FRC
	cd t300s; make -f t300s.mk ${MFLAGS}

lib450: FRC
	cd t450; make -f t450.mk ${MFLAGS}

libvt0: FRC
	cd vt0; make -f vt0.mk ${MFLAGS}

libplotaed: FRC
	cd aed; make -f aed.mk ${MFLAGS}

libplotbg: FRC
	cd bitgraph; make -f bitgraph.mk ${MFLAGS}

libplotdumb: FRC
	cd dumb; make -f dumb.mk ${MFLAGS}

libplotgigi: FRC
	cd gigi; make -f gigi.mk ${MFLAGS}

libplot2648: FRC
	cd hp2648; make -f hp2648.mk ${MFLAGS}

libplot7221: FRC
	cd hp7221; make -f hp7221.mk ${MFLAGS}

libplotimagen: FRC
	cd imagen; make -f imagen.mk ${MFLAGS}

FRC:

clean:
	rm -f errs a.out core
	-for i in ${SUBDIRS}; do \
		(cd $$i; make -f $$i.mk clean); \
	done

clobber: 
	rm -f errs a.out core
	-for i in ${SUBDIRS}; do \
		(cd $$i; make -f $$i.mk clobber); \
	done
