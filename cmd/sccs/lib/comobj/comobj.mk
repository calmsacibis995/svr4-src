#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/comobj/comobj.mk	1.16"
#
#

AR = ar
LORDER = lorder
ROOT=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT/usr/ccs/lib
INCBASE=../../hdr
CFLAGS = -O

LIBRARY = ../comobj.a

PRODUCTS = $(LIBRARY)

LLIBRARY = llib-lcom.ln

LINT = lint

LINTFLAGS = 

INC=
INCSYS=

SGSBASE=../../..
INCLIST=-I$(SGSBASE)/sgs/inc/common

LFILES = auxf.ln		\
		chkid.ln	\
		chksid.ln	\
		date_ab.ln	\
		date_ba.ln	\
		del_ab.ln	\
		del_ba.ln	\
		dodelt.ln	\
		dofile.ln	\
		dohist.ln	\
		doie.ln		\
		dolist.ln	\
		encode.ln	\
		eqsid.ln	\
		flushto.ln	\
		fmterr.ln	\
		getline.ln	\
		logname.ln	\
		newstats.ln	\
		permiss.ln	\
		pf_ab.ln	\
		putline.ln	\
		rdmod.ln	\
		setup.ln	\
		sid_ab.ln	\
		sid_ba.ln	\
		sidtoser.ln	\
		sinit.ln	\
		stats_ab.ln

FILES = auxf.o		\
		chkid.o		\
		chksid.o	\
		date_ab.o	\
		date_ba.o	\
		del_ab.o	\
		del_ba.o	\
		dodelt.o	\
		dofile.o	\
		dohist.o	\
		doie.o		\
		dolist.o	\
		encode.o	\
		eqsid.o		\
		flushto.o	\
		fmterr.o	\
		getline.o	\
		logname.o	\
		newstats.o	\
		permiss.o	\
		pf_ab.o		\
		putline.o	\
		rdmod.o		\
		setup.o		\
		sid_ab.o	\
		sid_ba.o	\
		sidtoser.o	\
		sinit.o		\
		stats_ab.o

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

$(LIBRARY): $(FILES)
	$(AR) cr $(LIBRARY) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIBRARY)

install:	$(LIBRARY)

clean:
	-rm -f *.o
	-rm -f lint.out
	-rm -f *.ln

clobber: clean
	-rm -f $(PRODUCTS)

lintit: $(LLIBRARY)
	@echo "Library $(LLIBRARY) is up to date\n"

$(LLIBRARY): $(LFILES)
	rm -f $(LLIBRARY)
	$(LINT) $(CFLAGS) $(LINTFLAGS) *.ln -o com >> lint.out
	ln -f $(LLIBRARY) ../$(LLIBRARY)

.SUFFIXES : .o .c .e .r .f .y .yr .ye .l .s .ln

.c.o:
	$(CC) $(CFLAGS) $(INCLIST) -c $<

.c.ln:
	echo "$<:" >> lint.out
	$(LINT) $(CFLAGS) $(LINTFLAGS) $(INCLIST) $< -c >> lint.out

auxf.ln:	$(INCBASE)/defines.h auxf.c
chkid.ln:	$(INCBASE)/defines.h chkid.c
chksid.ln:	$(INCBASE)/defines.h chksid.c
date_ab.ln:	date_ab.c
date_ba.ln:	$(INCBASE)/defines.h date_ba.c
del_ab.ln:	$(INCBASE)/defines.h del_ab.c
del_ba.ln:	$(INCBASE)/defines.h del_ba.c
dodelt.ln:	$(INCBASE)/defines.h dodelt.c
dofile.ln:	$(INCBASE)/defines.h dofile.c
dohist.ln:	$(INCBASE)/defines.h $(INCBASE)/had.h dohist.c
doie.ln:	$(INCBASE)/defines.h doie.c
dolist.ln:	$(INCBASE)/defines.h dolist.c
encode.ln:	$(INCBASE)/defines.h encode.c
eqsid.ln:	$(INCBASE)/defines.h eqsid.c
flushto.ln:	$(INCBASE)/defines.h flushto.c
fmterr.ln:	$(INCBASE)/defines.h fmterr.c
getline.ln:	$(INCBASE)/defines.h getline.c
logname.ln:	logname.c
newstats.ln:	$(INCBASE)/defines.h newstats.c
permiss.ln:	$(INCBASE)/defines.h permiss.c
pf_ab.ln:	$(INCBASE)/defines.h pf_ab.c
putline.ln:	$(INCBASE)/defines.h putline.c
rdmod.ln:	$(INCBASE)/defines.h rdmod.c
setup.ln:	$(INCBASE)/defines.h setup.c
sid_ab.ln:	$(INCBASE)/defines.h sid_ab.c
sid_ba.ln:	$(INCBASE)/defines.h sid_ba.c
sidtoser.ln:	$(INCBASE)/defines.h sidtoser.c
sinit.ln:	$(INCBASE)/defines.h sinit.c
stats_ab.ln:	$(INCBASE)/defines.h stats_ab.c

auxf.o:		$(INCBASE)/defines.h auxf.c
chkid.o:	$(INCBASE)/defines.h chkid.c
chksid.o:	$(INCBASE)/defines.h chksid.c
date_ab.o:	date_ab.c
date_ba.o:	$(INCBASE)/defines.h date_ba.c
del_ab.o:	$(INCBASE)/defines.h del_ab.c
del_ba.o:	$(INCBASE)/defines.h del_ba.c
dodelt.o:	$(INCBASE)/defines.h dodelt.c
dofile.o:	$(INCBASE)/defines.h dofile.c
dohist.o:	$(INCBASE)/defines.h $(INCBASE)/had.h dohist.c
doie.o:		$(INCBASE)/defines.h doie.c
dolist.o:	$(INCBASE)/defines.h dolist.c
encode.o:	$(INCBASE)/defines.h encode.c
eqsid.o:	$(INCBASE)/defines.h eqsid.c
flushto.o:	$(INCBASE)/defines.h flushto.c
fmterr.o:	$(INCBASE)/defines.h fmterr.c
getline.o:	$(INCBASE)/defines.h getline.c
logname.o:	logname.c
newstats.o:	$(INCBASE)/defines.h newstats.c
permiss.o:	$(INCBASE)/defines.h permiss.c
pf_ab.o:	$(INCBASE)/defines.h pf_ab.c
putline.o:	$(INCBASE)/defines.h putline.c
rdmod.o:	$(INCBASE)/defines.h rdmod.c
setup.o:	$(INCBASE)/defines.h setup.c
sid_ab.o:	$(INCBASE)/defines.h sid_ab.c
sid_ba.o:	$(INCBASE)/defines.h sid_ba.c
sidtoser.o:	$(INCBASE)/defines.h sidtoser.c
sinit.o:	$(INCBASE)/defines.h sinit.c
stats_ab.o:	$(INCBASE)/defines.h stats_ab.c
