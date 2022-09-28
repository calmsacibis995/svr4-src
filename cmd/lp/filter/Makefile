#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/Makefile	1.14.4.1"
#
# Makefile for lp/filter
#



TOP	=	..

include ../common.mk


DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC)

LDFLAGS = -s

TABLES	=	435_table 455_table 473_table 475_table HP_table

FILTERS =	5310 hp2631a pprx prx 455_filter 473_filter \
		475_filter ATT_s_filter HP_filter

MISC	= 	slow.filter

all:		$(MISC) $(FILTERS)
	cd postscript; $(MAKE)

install:
	for misc in $(MISC) $(FILTERS) $(TABLES); \
	do \
		$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(LPBINDIR) $$misc; \
	done
	cd postscript; $(MAKE) install

5310:		5310.o
	$(CC) $(LDFLAGS) -o 5310 5310.o $(SHLIBS)

hp2631a:	hp2631a.o
	$(CC) $(LDFLAGS) -o hp2631a hp2631a.o $(SHLIBS)

prx:		prx.o
	$(CC) $(LDFLAGS) -o prx prx.o $(SHLIBS)

pprx:		pprx.o
	$(CC) $(LDFLAGS) -o pprx pprx.o $(SHLIBS)

455_filter:	455_filter.o
	$(CC) $(LDFLAGS) -o 455_filter 455_filter.o $(SHLIBS)

473_filter:	473_filter.o
	$(CC) $(LDFLAGS) -o 473_filter 473_filter.o $(SHLIBS)

475_filter:	475_filter.o
	$(CC) $(LDFLAGS) -o 475_filter 475_filter.o $(SHLIBS)

ATT_s_filter:	ATT_s_filter.o
	$(CC) $(LDFLAGS) -o ATT_s_filter ATT_s_filter.o $(SHLIBS)

HP_filter:	HP_filter.o
	$(CC) $(LDFLAGS) -o HP_filter HP_filter.o $(SHLIBS)

clean clobber:
	cd postscript; $(MAKE) $@
	-rm -f 455_filter 455_filter.o 473_filter 473_filter.o 475_filter 475_filter.o
	-rm -f ATT_s_filter ATT_s_filter.o HP_filter HP_filter.o 5310 5310.o
	-rm -f hp2631a hp2631a.o pprx pprx.o prx prx.o

strip:

lint:
