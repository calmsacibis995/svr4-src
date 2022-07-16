#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libm:i386/makefile	1.9"
#
# makefile for libm/i386
#
#
#

.SUFFIXES: .p

ROOT=
SGS=
OWN=bin
GRP=bin
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../../cmd/sgs
INS=$(SGSBASE)/sgs.install
INSDIR=
STRIP=strip
PRODUCTS=
HFILES=../port/fpparts.h ../port/synonyms.h
CCSLIB=$(ROOT)/usr/ccs/lib
DEFLIST=
MAKE=make
INSPDIR=
INCMATH=../port
INCLIST=-I$(INCMATH)
AR=ar
ANSIDEF=ansi.def
PROFMODE=
PMOVE=@#
LORDER=lorder
PORT=../port
M4=m4 ./m4.def
M4DEF="-DMCOUNT=/"

OBJECTS=\
acosh.o	asin.o	asinh.o		\
atanh.o	cbrt.o	erf.o	\
fabs.o	floor.o	fmod.o \
gamma.o	hypot.o sqrt.o \
j0.o	j1.o	jn.o	\
log1p.o matherr.o rint.o	\
pow.o	rem.o	sinh.o  tanh.o \
asinf.o	fabsf.o	\
floorf.o	fmodf.o	\
powf.o	sinhf.o	 \
tanhf.o \
atan.o exp.o log.o reduce.o \
sin.o atanf.o expf.o logf.o \
reducef.o sinf.o sqrtf.o


POBJECTS=\
acosh.p	asin.p	asinh.p	atanh.p	cbrt.p	erf.p \
fabs.p	floor.p	fmod.p	gamma.p	hypot.p sqrt.p  \
j0.p	j1.p	jn.p	log1p.p matherr.p rint.p	\
pow.p	rem.p	sinh.p  tanh.p  asinf.p	fabsf.p	\
floorf.p fmodf.p powf.p	sinhf.p	 \
tanhf.p  atan.p exp.p log.p reduce.p \
sin.p atanf.p expf.p logf.p reducef.p sinf.p sqrtf.p

NONANS=\
acosh.o	asin.o	asinh.o		\
atanh.o	cbrt.o	erf.o	\
fabs.o	floor.o	fmod.o \
gamma.o	hypot.o sqrt.o \
j0.o	j1.o	jn.o	\
log1p.o matherr.o rint.o	\
pow.o	rem.o	sinh.o  tanh.o \
atan.o exp.o log.o reduce.o sqrt.o

PNONANS=\
acosh.p	asin.p	asinh.p		\
atanh.p	cbrt.p	erf.p	\
fabs.p	floor.p	fmod.p \
gamma.p	hypot.p sqrt.p \
j0.p	j1.p	jn.p	\
log1p.p matherr.p rint.p	\
pow.p	rem.p	sinh.p  tanh.p \
atan.p exp.p log.p reduce.p sqrt.p

all:	
	$(MAKE) prof PROFMODE=-p PMOVE= ANSIDEF=$(ANSIDEF) M4DEF=mcount.def
	$(MAKE) nonprof PROFMODE= PMOVE=@# ANSIDEF=$(ANSIDEF)

nonansi:	
	$(MAKE) nonansiprof PROFMODE=-p PMOVE= ANSIDEF=$(ANSIDEF) M4DEF=mcount.def
	$(MAKE) nonansinoprof PROFMODE= PMOVE=@# ANSIDEF=$(ANSIDEF)


nonprof:	maketemp $(OBJECTS)

prof:	maketemp $(POBJECTS)

nonansiprof: maketemp $(PNONANS)

nonansinoprof:	maketemp $(NONANS)

maketemp:
	-rm -rf tmp
	mkdir tmp
	
archive:	all profarch nonprofarch

nonansi_arch:	nonansi nonansi_profarch nonansi_nonprofarch
	#
	#
nonprofarch:	$(OBJECTS)
	#
	$(LORDER) *.o | tsort >objlist 
	# build the archive with the modules in correct order.
	$(AR) q lib.libm `cat objlist` 
	#
profarch:	$(POBJECTS)
	# build the profiled library archive, first renaming the
	# .p (profiled object) modules to .o
	-rm -rf pobjects
	mkdir pobjects
	for i in *.p ; do cp $$i pobjects/`basename $$i .p`.o ; done
	$(LORDER) pobjects/*.o | tsort >objlist 
	$(AR) q libp.libm `cat objlist` 
	-rm -rf pobjects

nonansi_nonprofarch:	$(NONANS)
	#
	$(LORDER) *.o | tsort >objlist 
	# build the archive with the modules in correct order.
	$(AR) q lib.libm `cat objlist` 
	#
	#
nonansi_profarch:	$(PNONANS)
	# build the profiled library archive, first renaming the
	# .p (profiled object) modules to .o
	-rm -rf pobjects
	mkdir pobjects
	for i in *.p ; do cp $$i pobjects/`basename $$i .p`.o ; done
	$(LORDER) pobjects/*.o | tsort >objlist 
	$(AR) q libp.libm `cat objlist` 
	-rm -rf pobjects
	#
	#

move:	nonprofmove profmove
	#

nonprofmove:
	# move the library or libraries into the correct directory
	sh $(INS) 644 $(OWN) $(GRP) $(INSDIR)/libm.a lib.libm ; \
	rm -f lib.libm
profmove:
	if [ ! -d $(INSPDIR) ]; then \
	mkdir $(INSPDIR); \
	fi
	sh $(INS) 644 $(OWN) $(GRP) $(INSPDIR)/libm.a libp.libm ; \
	# delete temporary libraries
	-rm -f libp.libm

clean:
	-rm -rf *.[op] tmp
	-rm -rf lib*.contents objlist pobjects

clobber: clean
	-rm -rf *.[op] tmp
	-rm -rf lib*.libm lib*.contents objlist

acosh.o acosh.p:	$(PORT)/acosh.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/acosh.c 
	$(PMOVE)mv `basename $@ .p`.o $@

asin.o asin.p:	$(PORT)/asin.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/asin.c
	$(PMOVE)mv `basename $@ .p`.o $@

asinh.o asinh.p:	$(PORT)/asinh.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/asinh.c
	$(PMOVE)mv `basename $@ .p`.o $@

atanh.o atanh.p:	$(PORT)/atanh.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/atanh.c
	$(PMOVE)mv `basename $@ .p`.o $@

cbrt.o cbrt.p:	$(PORT)/cbrt.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/cbrt.c
	$(PMOVE)mv `basename $@ .p`.o $@

erf.o erf.p:	$(PORT)/erf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/erf.c
	$(PMOVE)mv `basename $@ .p`.o $@

fabs.o fabs.p:	$(PORT)/fabs.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/fabs.c
	$(PMOVE)mv `basename $@ .p`.o $@

floor.o floor.p:	$(PORT)/floor.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/floor.c
	$(PMOVE)mv `basename $@ .p`.o $@

fmod.o fmod.p:	fmod.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) fmod.c
	$(PMOVE)mv `basename $@ .p`.o $@

gamma.o gamma.p:	$(PORT)/gamma.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/gamma.c
	$(PMOVE)mv `basename $@ .p`.o $@

hypot.o hypot.p:	$(PORT)/hypot.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/hypot.c
	$(PMOVE)mv `basename $@ .p`.o $@

sqrt.o sqrt.p:	sqrt.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) sqrt.c
	$(PMOVE)mv `basename $@ .p`.o $@

j0.o j0.p:	$(PORT)/j0.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/j0.c
	$(PMOVE)mv `basename $@ .p`.o $@

j1.o j1.p:	$(PORT)/j1.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/j1.c
	$(PMOVE)mv `basename $@ .p`.o $@

jn.o jn.p:	$(PORT)/jn.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/jn.c
	$(PMOVE)mv `basename $@ .p`.o $@

log1p.o log1p.p:	$(PORT)/log1p.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/log1p.c
	$(PMOVE)mv `basename $@ .p`.o $@

matherr.o matherr.p:	$(PORT)/matherr.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/matherr.c
	$(PMOVE)mv `basename $@ .p`.o $@

rint.o rint.p:	$(PORT)/rint.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/rint.c
	$(PMOVE)mv `basename $@ .p`.o $@

pow.o pow.p:	$(PORT)/pow.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/pow.c
	$(PMOVE)mv `basename $@ .p`.o $@

rem.o rem.p:	rem.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) rem.c
	$(PMOVE)mv `basename $@ .p`.o $@

sinh.o sinh.p:	$(PORT)/sinh.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/sinh.c
	$(PMOVE)mv `basename $@ .p`.o $@

tanh.o tanh.p:	$(PORT)/tanh.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/tanh.c
	$(PMOVE)mv `basename $@ .p`.o $@

atan.o atan.p:	atan.s
	$(M4) $(M4DEF) $(ANSIDEF) atan.s > tmp/atan.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/atan.s
	$(PMOVE)mv `basename $@ .p`.o $@

exp.o exp.p:	exp.s
	$(M4) $(M4DEF) $(ANSIDEF) exp.s > tmp/exp.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/exp.s
	$(PMOVE)mv `basename $@ .p`.o $@

log.o log.p:	log.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) log.c
	$(PMOVE)mv `basename $@ .p`.o $@

reduce.o reduce.p:	reduce.s
	$(M4) $(M4DEF) $(ANSIDEF) reduce.s > tmp/reduce.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/reduce.s
	$(PMOVE)mv `basename $@ .p`.o $@

sin.o sin.p:	sin.s
	$(M4) $(M4DEF) $(ANSIDEF) sin.s > tmp/sin.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/sin.s
	$(PMOVE)mv `basename $@ .p`.o $@

asinf.o asinf.p:	$(PORT)/asinf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/asinf.c
	$(PMOVE)mv `basename $@ .p`.o $@

fabsf.o fabsf.p:	$(PORT)/fabsf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/fabsf.c
	$(PMOVE)mv `basename $@ .p`.o $@

floorf.o floorf.p:	$(PORT)/floorf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/floorf.c
	$(PMOVE)mv `basename $@ .p`.o $@

fmodf.o fmodf.p:	fmodf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) fmodf.c
	$(PMOVE)mv `basename $@ .p`.o $@

sqrtf.o sqrtf.p:	sqrtf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) sqrtf.c
	$(PMOVE)mv `basename $@ .p`.o $@

powf.o powf.p:	$(PORT)/powf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/powf.c
	$(PMOVE)mv `basename $@ .p`.o $@

sinhf.o sinhf.p:	$(PORT)/sinhf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/sinhf.c
	$(PMOVE)mv `basename $@ .p`.o $@

tanhf.o tanhf.p:	$(PORT)/tanhf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) $(PORT)/tanhf.c
	$(PMOVE)mv `basename $@ .p`.o $@

atanf.o atanf.p:	atanf.s
	$(M4) $(M4DEF) $(ANSIDEF) atanf.s > tmp/atanf.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/atanf.s
	$(PMOVE)mv `basename $@ .p`.o $@

expf.o expf.p:	expf.s
	$(M4) $(M4DEF) $(ANSIDEF) expf.s > tmp/expf.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/expf.s
	$(PMOVE)mv `basename $@ .p`.o $@

logf.o logf.p:	logf.c
	$(CC) -c $(CFLAGS) $(PROFMODE) $(DEFLIST) $(INCLIST) $(CMODE) logf.c
	$(PMOVE)mv `basename $@ .p`.o $@

reducef.o reducef.p:	reducef.s
	$(M4) $(M4DEF) $(ANSIDEF) reducef.s > tmp/reducef.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/reducef.s
	$(PMOVE)mv `basename $@ .p`.o $@

sinf.o sinf.p:	sinf.s
	$(M4) $(M4DEF) $(ANSIDEF) sinf.s > tmp/sinf.s
	$(CC) $(CFLAGS) $(PROFMODE) $(CMODE)  -c tmp/sinf.s
	$(PMOVE)mv `basename $@ .p`.o $@

