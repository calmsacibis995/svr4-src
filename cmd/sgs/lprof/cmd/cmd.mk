#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lxprof:cmd/cmd.mk	1.9.1.10"

PROF_SAVE	=
XPROF_INCS	=
PLBBASE		= ../libprof
INCBASE		= ../hdr

ROOT		=
SGS		= m32
OWN		= bin
GRP		= bin
CC		= cc
CFLAGS		= -O
LDLIBS		= 
LINT		= lint
LINTFLAGS 	=
LINK_MODE	=

PCFLAGS		= $(CFLAGS) -DTARGETPROFILER=1

BIN		= $(ROOT)/bin
LIB		= $(ROOT)/lib
USRBIN		= $(ROOT)/usr/bin
USRLIB		= $(ROOT)/usr/lib
CCSBIN		= $(ROOT)/usr/ccs/bin
CCSLIB		= $(ROOT)/usr/ccs/lib

SGSBASE		= ../..
CMDBASE		= ../../..
MACHINC		= $(SGSBASE)/inc/$(MACH)
INS		= $(SGSBASE)/sgs.install
INSDIR		= $(CCSBIN)
STRIP		= strip
HFILES		= \
		env.h			\
		glob.h 			\
		coredefs.h 		\
		cor_errs.h		\
		$(MACHINC)/sgs.h	\
		$(INCBASE)/retcode.h	\
		$(INCBASE)/funcdata.h	\
		$(INCBASE)/covfile.h	\
		$(INCBASE)/filedata.h	\
		$(PLBBASE)/symint.h	\
		$(PLBBASE)/symintHdr.h
INCDIRS		= \
		-I . \
		-I ../hdr \
		-I ../libprof \
		-I $(SGSBASE)/inc/common \
		-I $(MACHINC) \
		$(XPROF_INCS)
SOURCES		= merge.c rept_utl.c src_list.c utility.c \
		lin_rept.c sum.c list_rept.c main.c
OBJECTS		= merge.o rept_utl.o src_list.o utility.o \
		lin_rept.o sum.o list_rept.o main.o
PRODUCTS	= ../lprof

LIBELF		= $(SGSBASE)/libelf/$(MACH)/libelf.a
PLBBASE		= $(SGSBASE)/lprof/libprof
LDFLAGS		=
PROFLIBD	= $(PLBBASE)
LIBSYMINT	= -L$(PROFLIBD) -lsymint
LIBPROF		= -L$(PROFLIBD) -lprof
LIBS		= $(LIBPROF) $(LIBELF)

ENVPARMS	= \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" \ LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LIBELF="$(LIBELF)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" \
	LINK_MODE="$(LINK_MODE)" \
	PROF_SAVE="$(PROF_SAVE)" LIBS="$(LIBS)" 


all:	$(PRODUCTS)

$(PRODUCTS): $(PLBBASE)/libprof.a $(OBJECTS)
	$(CC) -o $(PRODUCTS) $(PCFLAGS) $(OBJECTS) \
	$(LDFLAGS) $(LDLIBS) $(LIBS) $(LINK_MODE)

main.o:	$(HFILES) main.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

lin_rept.o: $(HFILES) lin_rept.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

sum.o: $(HFILES) sum.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

list_rept.o: $(HFILES) list_rept.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

merge.o: $(HFILES) merge.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

rept_utl.o: $(HFILES) rept_utl.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

src_list.o: $(HFILES) src_list.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

utility.o: $(HFILES) utility.c
	$(CC) -c $(PCFLAGS) $(INCDIRS) $*.c

install: all
	cp $(PRODUCTS) lprof.bak
	$(STRIP) $(PRODUCTS)
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(CCSBIN)/$(SGS)lprof $(PRODUCTS)
	mv lprof.bak $(PRODUCTS)

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)

$(PLBBASE)/libprof.a:
	cd $(PLBBASE); $(MAKE) -f libprof.mk $(ENVPARMS)

