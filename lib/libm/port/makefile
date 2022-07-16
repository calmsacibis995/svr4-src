#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libm:port/makefile	1.3"
#
# makefile for libm/port
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

.SUFFIXES: .p

M4=m4
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys
INCMATH=../port
DEFLIST=
INCLIST= -I$(INC) -I$(INCSYS) -I$(INCMATH)
CC=
AR=ar
LINT=lint
CFLAGS=-O
PROF=
NONPROF=
LFLAGS=

CFILES=\
acosh.c	asin.c	asinh.c	atan.c	atanh.c	cbrt.c	erf.c	\
exp.c	fabs.c	floor.c	fmod.c	gamma.c	hypot.c \
j0.c	j1.c	jn.c	log.c	log1p.c	matherr.c rint.c	\
pow.c	rem.c	sin.c   sinh.c \
sqrt.c	reduce.c	tanh.c \
asinf.c	atanf.c	expf.c	fabsf.c	floorf.c	fmodf.c	logf.c \
powf.c	sinf.c	sinhf.c	sqrtf.c	reducef.c	tanhf.c

OBJECTS=\
acosh.o	asin.o	asinh.o	atan.o	atanh.o	cbrt.o	erf.o	\
exp.o	fabs.o	floor.o	fmod.o	gamma.o	hypot.o \
j0.o	j1.o	jn.o	log.o	log1p.o	matherr.o rint.o	\
pow.o	rem.o	sin.o   sinh.o \
sqrt.o	reduce.o	tanh.o	\
asinf.o	atanf.o	expf.o	fabsf.o	floorf.o	fmodf.o	logf.o \
powf.o	sinf.o	sinhf.o	sqrtf.o	reducef.o	tanhf.o

POBJECTS=\
acosh.p	asin.p	asinh.p	atan.p	atanh.p	cbrt.p	erf.p	\
exp.p	fabs.p	floor.p	fmod.p	gamma.p	hypot.p \
j0.p	j1.p	jn.p	log.p	log1p.p	matherr.p rint.p	\
pow.p	rem.p	sin.p	sinh.p  \
sqrt.p	reduce.p	tanh.p	\
asinf.p	atanf.p	expf.p	fabsf.p	floorf.p	fmodf.p	logf.p \
powf.p	sinf.p	sinhf.p	sqrtf.p	reducef.p	tanhf.p

all:
	$(PROF)make -e $(IGN) prof NONPROF=@#
	$(NONPROF)make -e $(IGN) nonprof PROF=@#

nonprof: $(OBJECTS)

prof: $(POBJECTS)

$(OBJECTS):	$(INCMATH)/fpparts.h $(INC)/math.h $(INC)/values.h \
			$(INC)/errno.h $(INCSYS)/errno.h
$(POBJECTS):	$(INCMATH)/fpparts.h $(INC)/math.h $(INC)/values.h \
			$(INC)/errno.h $(INCSYS)/errno.h

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(CFLAGS) $(INCLIST) $(DEFLIST) -c $*.c  
	$(PROF)@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) $(INCLIST) $(DEFLIST) -c $*.c  && mv $(*F).o $*.p

clean:
	-rm -f *.[op]

clobber: clean
	-rm -f *.[op]

lintit:	$(CFILES)
	$(LINT) $(LFLAGS) $(DEFLIST) $(INCLIST) $(CFILES) > lint.out
