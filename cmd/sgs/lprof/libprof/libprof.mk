#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libprof:libprof.mk	1.7.1.10"
#
#	makefile for libprof.a and for libsymint.a
#

PROF_RUNLD=ld
PROF_PCRTI=
PROF_PCRTN=
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
AR		= ar
ARFLAGS		= -r
LORDER		= lorder

# TARGETPROFILER:   1 => lprof   2 => prof
CFLAGS_1	= $(CFLAGS) -DTARGETPROFILER=1
CFLAGS_2	= $(CFLAGS) -DTARGETPROFILER=2

BIN		= $(ROOT)/bin
LIB		= $(ROOT)/lib
USRBIN		= $(ROOT)/usr/bin
USRLIB		= $(ROOT)/usr/lib
CCSBIN		= $(ROOT)/usr/ccs/bin
CCSLIB		= $(ROOT)/usr/ccs/lib

CMDBASE		= ../../..
SGSBASE		= ../..
MACHINC		= $(SGSBASE)/inc/$(MACH)
INS		= $(SGSBASE)/sgs.install
INSDIR		= $(CCSBIN)
STRIP		= strip

HFILES_A	= lst_str.h cov_errs.h \
		$(INCBASE)/covfile.h $(INCBASE)/retcode.h $(INCBASE)/filedata.h 
HFILES_B	= symint.h symintHdr.h debug.h profopt.h
HFILES		= $(HFILES_A) $(HFILES_B)

INCDIRS		= \
		-I . \
		-I ../hdr \
		-I $(SGSBASE)/inc/common \
		-I $(MACHINC) \
		$(XPROF_INCS)

PROD_1		= libprof.a
PROD_2		= libsymint.a
PROD_1SO	= libprof.so
PROD_2SO	= libsymint.so
PRODUCTS	= $(PROD_1) $(PROD_2)

SRCS_1		= dump.c comops.c cov_join.c exist.c new.c soqueue.c
SRCS_2		= symintClose.c symintErr.c symintLoad.c \
		symintOpen.c symintUtil.c
SOURCES		= $(SRCS_1) $(SRCS_2)

IOBJS_0		= symintClose.o symintErr.o symintLoad.o \
		symintOpen.o symintUtil.o
IOBJS_1		= symintClose1.o symintErr1.o symintLoad1.o \
		symintOpen1.o symintUtil1.o
IOBJS_2		= symintClose2.o symintErr2.o symintLoad2.o \
		symintOpen2.o symintUtil2.o
OBJS_1		= $(IOBJS_1) dump.o comops.o cov_join.o exist.o new.o soqueue.o
OBJS_2		= $(IOBJS_2)
OBJECTS		= $(OBJS_1) $(OBJS_2) $(IOBJS_0)

LIBELF		= $(SGSBASE)/libelf/$(MACH)/libelf.a
PLBBASE		= $(SGSBASE)/lprof/libprof
PROFLIBD	= $(PLBBASE)
LIBSYMINT	= -L$(PROFLIBD) -lsymint
LDFLAGS		=
LIBS		= $(PLBBASE)/libprof.a $(LIBELF)

ENVPARMS	= \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" \ LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LIBELF="$(LIBELF)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" \
	LINK_MODE="$(LINK_MODE)" \
	PROF_SAVE="$(PROF_SAVE)" LIBS="$(LIBS)" 


all: $(PRODUCTS)
	if test ! "$(PROF_SAVE)"; then rm -f *.o; fi

$(PROD_1): $(OBJS_1)
	$(AR) $(ARFLAGS) $(PROD_1) `$(LORDER) $(OBJS_1) | tsort`

$(PROD_2): $(OBJS_2)
	$(AR) $(ARFLAGS) $(PROD_2) `$(LORDER) $(OBJS_2) | tsort`

$(PROD_1SO): $(PROD_1) $(OBJS_1)
	if echo $(CFLAGS) | grep ql > /dev/null; then \
		$(PROF_RUNLD) -dy -G -Bsymbolic $(OBJS_1) \
			$(PROF_PCRTI) $(PROF_PCRTN) \
			-lelf -lc -lm -Qy -o $(PROD_1SO); \
	else \
		$(PROF_RUNLD) -dy -G -Bsymbolic $(OBJS_1) \
			-lelf -lc -lm -Qy -o $(PROD_1SO); \
	fi

$(PROD_2SO): $(PROD_1) $(OBJS_2)
	if echo $(CFLAGS) | grep ql > /dev/null; then \
		$(PROF_RUNLD) -dy -G -Bsymbolic $(OBJS_2) \
			$(PROF_PCRTI) $(PROF_PCRTN) \
			-lelf -lc -lm -Qy -o $(PROD_2SO); \
	else \
		$(PROF_RUNLD) -dy -G -Bsymbolic $(OBJS_2) \
			-lelf -lc -lm -Qy -o $(PROD_2SO); \
	fi

comops.o:	$(HFILES) comops.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c
cov_join.o:	$(HFILES) cov_join.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c
dump.o:		$(HFILES) dump.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c
exist.o:	$(HFILES) exist.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c
new.o:		$(HFILES) new.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c
soqueue.o:	$(HFILES) soqueue.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c

symintClose1.o symintClose2.o: symintClose.o
symintClose.o: $(HFILES_B) symintClose.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c;	mv $*.o $*1.o
	$(CC) -c $(CFLAGS_2) $(INCDIRS) $*.c;	ln $*.o $*2.o

symintErr1.o symintErr2.o: symintErr.o
symintErr.o: $(HFILES_B) symintErr.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c;	mv $*.o $*1.o
	$(CC) -c $(CFLAGS_2) $(INCDIRS) $*.c;	ln $*.o $*2.o

symintLoad1.o symintLoad2.o: symintLoad.o
symintLoad.o: $(HFILES_B) symintLoad.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c;	mv $*.o $*1.o
	$(CC) -c $(CFLAGS_2) $(INCDIRS) $*.c;	ln $*.o $*2.o

symintOpen1.o symintOpen2.o: symintOpen.o
symintOpen.o: $(HFILES_B) symintOpen.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c;	mv $*.o $*1.o
	$(CC) -c $(CFLAGS_2) $(INCDIRS) $*.c;	ln $*.o $*2.o

symintUtil1.o symintUtil2.o: symintUtil.o
symintUtil.o: $(HFILES_B) symintUtil.c
	$(CC) -c $(CFLAGS_1) $(INCDIRS) $*.c;	mv $*.o $*1.o
	$(CC) -c $(CFLAGS_2) $(INCDIRS) $*.c;	ln $*.o $*2.o


install: all
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/libprof.a libprof.a

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)
	rm -f $(PROD_1SO) $(PROD_2SO)

