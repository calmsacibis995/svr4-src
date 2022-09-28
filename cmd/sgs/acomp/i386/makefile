#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)acomp:i386/makefile	1.18"

#	i386 machine-dependent makefile for
#	ANSI C compiler.

# Standard environmental stuff.

ROOT=
SGS=	i386
OWN=	bin
GRP=	bin
CC=	cc
CFLAGS=	-O
LDLIBS=
LINK_MODE=
LINT=	lint
LINTFLAGS=
o=o
SGSBASE= ../..
MACH=	i386
MACHINC= $(SGSBASE)/inc/$(MACH)
CCSLIB=	$(ROOT)/usr/ccs/lib
INS=	$(SGSBASE)/sgs.install
INSDIR=	$(CCSLIB)
STRIP=	strip

# Use i386 for machine-dependent information (sgs.h)
I386INC= $(MACHINC)
COMINC=	$(SGSBASE)/inc/common
# lint requires SGSBASE-relative path
ACOMP=	acomp
COMDIR=	$(SGSBASE)/$(ACOMP)/common
MDPDIR=	$(SGSBASE)/$(ACOMP)/i386
INC=	$(ROOT)/usr/include
# DFLTINC is the default ./usr/include directory used by acpp
#
DFLTINC=	$(ROOT)/usr/include
LD=	ld
LEX=	lex
LFLAGS=
YACC=	yacc
YFLAGS=	-l

CG=	$(SGSBASE)/cg
TARGET=	i386
CG_MDP=	$(CG)/$(TARGET)
CG_O=	$(CG_MDP)/cg.o
ACC_MDP=	../i386
COMPVERS=	'"01.01"'
CV=	COMPVERS=-DCOMPVERS=$(COMPVERS)
ACOMPMAKEFILE=	$(COMDIR)/acomp.mk

# Enable handling of enhanced asm's
INLINE=	-DIN_LINE
# Enable CG's use of floating point emulator on Amdahl host.
FP_EMULATE=	-DFP_EMULATE

# MERGED_CPP enables a merged preprocessor/compiler
# Several things must be changed to enable/disable a
# merged preprocessor.
# To enable:
# MERGED_CPP=	-DMERGED_CPP=0		turns support on (selects acomp, too)
# ACLEX_O=	lex.$o			selects lexical interface
# CPP_O=	<location of acpp.o>	partially linked .o for CPP
# CPP_INC=	-I$(CPP_COMMON)		enables search of directory
# CPP_INTERFACE= $(CPP_COMMON)/interface.h for dependency
#
# Corresponding values to enable a standalone scanner are:
# MERGED_CPP=				turn support off
# ACLEX_O=	aclex.$o		select standalone scanner
# CPP_O=				no CPP object file
# CPP_INC=				no search of other directory
# CPP_INTERFACE=			no extra dependency
#
CPP=		$(SGSBASE)/acpp
CPP_COMMON=	$(CPP)/common
CPP_MDP=	$(CPP)/i386
#
MERGED_CPP=	-DMERGED_CPP=0
ACLEX_O=	lex.$o
CPP_O=		$(CPP_MDP)/acpp.o
CPP_INC=	-I$(CPP_COMMON)
CPP_INTERFACE=	$(CPP_COMMON)/interface.h

# OPTIM_SUPPORT enables support for HALO optimizer
# OPTIM_SUPPORT=			turns support off
# OPTIM_SUPPORT= -DOPTIM_SUPPORT	turns support on
OPTIM_SUPPORT= -DOPTIM_SUPPORT

# RODATA enables support of a separate read-only data
# section for data and strings.
# RODATA=				turns support off
# RODATA= -DRODATA			turns support on
RODATA=		-DRODATA

# VOL_SUPPORT enables support for volatile objects in CG.
# VOL_SUPPORT=				turns support off
# VOL_SUPPORT= -DVOL_SUPPORT		turns support on
VOL_SUPPORT= -DVOL_SUPPORT

# YYDEBUG enables yacc debugging in the ANSI C grammar
# YYDEBUG=			support off if NODBG off
# YYDEBUG=	-DYYDEBUG	support on
YYDEBUG=

# include files when building lint (passed in):  LINT_H for
# dependencies, LINT_INC for -I options to cc.
# CXREF is used for making acpp.o
LINT_H=
LINT_INC=
LINT_MDP=
CXREF=

NODBG=	-DNODBG
DEFLIST=

# PASS... definitions get passed along to the next level of
# makefile.  Each ANSI C compiler builds the machine-dependent
# cg.o and the semi-machine-independent compiler from the
# common source.  A preprocessor portion may also be built if
# the preprocessor and compiler are a single process.

PASSCOMMON= \
	-$(MAKEFLAGS) ROOT="$(ROOT)" SGSBASE="$(SGSBASE)" SGS="$(SGS)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LD="$(LD)" LINK_MODE="$(LINK_MODE)" \
	LEX="$(LEX)" LFLAGS="$(LFLAGS)" YACC="$(YACC)" YFLAGS="$(YFLAGS)" \
	LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" \
	NODBG="$(NODBG)" INC="$(INC)" DFLTINC="$(DFLTINC)"
PASSDEFINE= \
	INLINE="$(INLINE)" FP_EMULATE="$(FP_EMULATE)" \
	VOL_SUPPORT="$(VOL_SUPPORT)" OPTIM_SUPPORT="$(OPTIM_SUPPORT)" \
	RODATA="$(RODATA)"
PASSCG=	$(PASSCOMMON) $(PASSDEFINE)
PASSACOMP= \
	$(PASSCOMMON) $(PASSDEFINE) \
	$(CV) TARGET="$(TARGET)" COMDIR="$(COMDIR)" ACC_MDP="$(ACC_MDP)" \
	CG_O="$(CG_O)" CG_INCS="-I$(CG_MDP) -I$(CG)/common" \
	MDPINC="$(I386INC)" COMINC="$(COMINC)" \
	YYDEBUG="$(YYDEBUG)" \
	MERGED_CPP="$(MERGED_CPP)" ACLEX_O='$(ACLEX_O:$o=$o)' CPP_O="$(CPP_O)" \
	CPP_INC="$(CPP_INC)" CPP_INTERFACE="$(CPP_INTERFACE)"
PASSCPP= $(PASSCOMMON) CXREF="$(CXREF)"
PASSLINT=	LINT_H="$(LINT_H)" DEFLIST=-DLINT \
		LINT_INC="$(LINT_INC)" $(PASSACOMP)

build all:	cg acpp
	$(MAKE) $(PASSACOMP) -f $(ACOMPMAKEFILE) all

cg $(CG_O):
	cd $(CG_MDP); $(MAKE) -f cg.mk $(PASSCG) cg.o

acpp $(CPP_O):
	if [ "$(CPP_O)" ]; then \
	    cd $(CPP_MDP); $(MAKE) -f acpp.mk $(PASSCPP) acpp.o; \
	fi

# target to build an acomp.o for lint
forlint:	acpp
	$(MAKE) $(PASSLINT) -f $(ACOMPMAKEFILE) forlint

clean:
	cd $(CG_MDP); $(MAKE) -f cg.mk $(PASSCG) clean
	$(MAKE) -f $(ACOMPMAKEFILE) $(PASSACOMP) clean
	if [ "$(CPP_O)" ]; then \
	    cd $(CPP_MDP); $(MAKE) -f acpp.mk $(PASSCPP) clean; \
	fi

clobber:
	cd $(CG_MDP); $(MAKE) -f cg.mk $(PASSCG) clobber
	$(MAKE) -f $(ACOMPMAKEFILE) $(PASSACOMP) clobber
	if [ "$(CPP_O)" ]; then \
	    cd $(CPP_MDP); $(MAKE) -f acpp.mk $(PASSCPP) clobber; \
	fi

shrink:		clobber


install:	$(INSDIR)/$(SGS)acomp

$(INSDIR)/$(SGS)acomp:	acomp
	cp acomp acomp.bak
	$(STRIP) acomp
	sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)acomp acomp
	mv acomp.bak acomp

lintit:
	exec >lint.out 2>&1;					\
	$(MAKE) $(PASSACOMP) -f $(ACOMPMAKEFILE) acomp.ln;	\
	cd $(CG_MDP); $(MAKE) -f cg.mk $(PASSCG) cg.ln;		\
	if [ "$(CPP_O)" ]; then					\
	    cd $(CPP_MDP);					\
	    $(MAKE) -f acpp.mk $(PASSCPP) acpp.ln;		\
	fi;							\
	cd $(MDPDIR);						\
	if [ "$(CPP_O)" ]; then					\
	    $(LINT) $(LINTFLAGS) acomp.ln $(CG_MDP)/cg.ln $(CPP_MDP)/acpp.ln; \
	else							\
	    $(LINT) $(LINTFLAGS) acomp.ln $(CG_MDP)/cg.ln;	\
	fi

llintit:
	$(MAKE) $(PASSLINT) -f $(ACOMPMAKEFILE) acomp.ln;	\
	if [ "$(CPP_O)" ]; then					\
	    cd $(CPP_MDP);					\
	    $(MAKE) -f acpp.mk $(PASSCPP) acpp.ln;		\
	fi;							\
	cd $(LINT_MDP);						\
	if [ "$(CPP_O)" ]; then 				\
	    cat acomp.ln cg.ln $(CPP_MDP)/acpp.ln > others.ln; 	\
	else 							\
	    cat acomp.ln cg.ln > others.ln;			\
	fi
