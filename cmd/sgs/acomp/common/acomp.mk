#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)acomp:common/acomp.mk	55.2"

#	Generic makefile for ANSI C Compiler

ROOT=
CC=		cc
CFLAGS=		-g
LD=		ld
o=o
LINT=		lint
LINTFLAGS=
SGSBASE= 	../..
COMINC=		$(SGSBASE)/inc/common
COMDIR=		../common
# Generic machine-dependent system-wide include (for sgs.h)
MDPINC=		$(SGSBASE)/inc/m32
INC=		$(ROOT)/usr/include
LINK_MODE=
LEX=		lex
LFLAGS=
YACC=		yacc
#YFLAGS=	-v
YFLAGS=

# YYDEBUG=		turns off yacc debug code
# YYDEBUG=-DYYDEBUG	turns on yacc debug code
YYDEBUG=	-DYYDEBUG
# NODBG=		enables internal debugging code
# NODBG=-DNODBG		suppresses internal debugging code
NODBG=
# FP_EMULATE affects manifest.h, gets passed in.
FP_EMULATE=	-DFP_EMULATE
# OPTIM_SUPPORT enables support for HALO optimizer
# OPTIM_SUPPORT=			turns support off
# OPTIM_SUPPORT= -DOPTIM_SUPPORT	turns support on
OPTIM_SUPPORT=	-DOPTIM_SUPPORT

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
#
MERGED_CPP=
ACLEX_O=	aclex.$o
CPP_O=
CPP_INC=
CPP_INTERFACE=

# include files when building lint (passed in):  LINT_H for
# dependencies, LINT_INC for -I options to cc.
LINT_H=
LINT_INC=

# Must derive y.tab.h from acgram.y
YACC_CMD=	$(YACC) $(YFLAGS) -d
DEFLIST=
INLINE=		-DIN_LINE
OPTIONS=	$(INLINE) $(OPTIM_SUPPORT) $(FP_EMULATE) $(MERGED_CPP) $(NODBG)
LIBS=	-ll
TARGET=		mau
CG=		$(SGSBASE)/cg
CG_MDP=		$(CG)/$(TARGET)
CG_O=		$(CG_MDP)/cg.o

# for debugging on Amdahl
#CG_O=	$(CG_MDP)/allo.o $(CG_MDP)/cgen.o $(CG_MDP)/comm2.o $(CG_MDP)/cost.o \
#	$(CG_MDP)/match.o $(CG_MDP)/reader.o $(CG_MDP)/xdefs.o $(CG_MDP)/local.o \
#	$(CG_MDP)/local2.o $(CG_MDP)/inline.o \
#	$(CG_MDP)/nail.o $(CG_MDP)/stasg.o $(CG_MDP)/table.o
CG_COMMON=	$(CG)/common
CG_INCS=	-I$(CG_MDP) -I$(SGSBASE)/cg/m32com -I$(CG_COMMON)
MFILE2=		$(CG_COMMON)/mfile2.h
# Define shorthand for directory of source.
A=	$(COMDIR)
# ACC machine dependent files
ACC_MDP=	.


ACC_INC=        -I$(ACC_MDP) -I$A $(CG_INCS) -I$(MDPINC)
INCLIST=      $(ACC_INC) $(LINT_INC)
COMPVERS=
CC_CMD=	$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) '$(COMPVERS)' $(OPTIONS)


# Splitting up the object files is a hack to work around bugs in
# the Amdahl loader, dealing with ld -r files.
OFILES=	$(OF1) $(OF2) $(OF3) $(ACLEX_O)
OF1=	acgram.$o ansisup.$o cgstuff.$o debug.$o decl.$o
OF2=	elfdebug.$o err.$o init.$o lexsup.$o main.$o optim.$o
OF3=	p1allo.$o sharp.$o stmt.$o sym.$o trees.$o types.$o
ACOMP_O=	$(OF2) $(CG_O) $(OF1) $(OF3) $(ACLEX_O)

ACCHDR=	$A/p1.h $A/aclex.h $A/ansisup.h $A/cgstuff.h \
	$A/debug.h $A/decl.h $A/err.h $A/host.h $A/init.h \
	$A/lexsup.h $A/node.h $A/optim.h $A/p1allo.h \
	$A/stmt.h $A/sym.h $A/target.h $A/tblmgr.h \
	$A/trees.h $A/types.h \
	$(ACC_MDP)/mddefs.h
HFILES=	$(ACCHDR) $(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/dwarf.h \
	$(MDPINC)/sgs.h
P1_H=	$(ACCHDR) $(CG_MDP)/macdefs.h $(CG_COMMON)/manifest.h $(LINT_H)

SOURCES= $A/acgram.y $A/aclex.l $A/ansisup.c $A/cgstuff.c $A/debug.c \
	$A/decl.c $A/elfdebug.c $A/err.c $A/init.c $A/lex.c $A/lexsup.c \
	$A/main.c $A/optim.c $A/p1allo.c $A/sharp.c $A/stmt.c $A/sym.c \
	$A/trees.c $A/types.c

OBJECTS= $A/acgram.o $A/aclex.o $A/ansisup.o $A/cgstuff.o $A/debug.o \
	$A/decl.o $A/elfdebug.o $A/err.o $A/init.o $A/lex.o $A/lexsup.o \
	$A/main.o $A/optim.o $A/p1allo.o $A/sharp.o $A/stmt.o $A/sym.o \
	$A/trees.o $A/types.o

PRODUCTS= acomp




all build:		acomp

acomp.o:		$(ACOMP_O)
			$(LD) -r -o acomp.o $(ACOMP_O)

forlint:		acompcpp.$o

acompcpp.o:		$(ACOMP_O) $(CPP_O)
			$(LD) -r -o acompcpp.o $(ACOMP_O) $(CPP_O)

acomp:			$(ACOMP_O) $(CPP_O)
			$(CC) -o acomp $(CFLAGS) $(LINK_MODE) \
				$(ACOMP_O) $(CPP_O) $(LIBS)

PASSLINT=		CC="$(LINT)" CFLAGS="$(CFLAGS) $(LINTFLAGS)" o=ln \
			DEFLIST="$(DEFLIST)" INCLIST="$(INCLIST)" \
			NODBG="$(NODBG)" YYDEBUG="$(YYDEBUG)" \
			OPTIM_SUPPORT="$(OPTIM_SUPPORT)" \
			COMDIR="$(COMDIR)" COMINC="$(COMINC)" LIBS="$(LIBS)" \
			CG="$(CG)" TARGET="$(TARGET)" CG_MDP="$(CG_MDP)" \
			CPP_INC="$(CPP_INC)" CPP_INTERFACE="$(CPP_INTERFACE)" \
			ACLEX_O='$(ACLEX_O:$o=$o)' MERGED_CPP="$(MERGED_CPP)" \
			ACC_MDP="$(ACC_MDP)"

lintit:
			$(MAKE) -$(MAKEFLAGS) -f $(COMDIR)/acomp.mk \
				$(PASSLINT) lintp2

acomp.ln:		$(SOURCES) $(ACCHDR)
			$(MAKE) -$(MAKEFLAGS) -f $(COMDIR)/acomp.mk \
				$(PASSLINT) acomp_ln

acomp_ln:		$(OF1:$o=ln) $(OF2:$o=ln) $(OF3:$o=ln) $(ACLEX_O:$o=ln) 
			cat $(OF1:$o=ln) $(OF2:$o=ln) \
				$(OF3:$o=ln) $(ACLEX_O:$o=ln) >acomp.ln

acgram.c:		$A/acgram.y
			$(YACC_CMD) $A/acgram.y
			mv y.tab.c acgram.c
			if cmp -s y.tab.h acgram.h; then \
			    rm y.tab.h; \
			else \
			    touch acgram.h.ok; \
			    cp y.tab.h acgram.h; \
			    rm y.tab.h; \
			fi

# Keeping this off the acgram.c dependency line prevents
# unnecessary rebuilds.  However, to accomplish that we
# need a hack in case acgram.h is missing altogether (but
# acgram.c is present).  In that case, run yacc to get it.
acgram.h:		acgram.h.ok
acgram.h.ok:		acgram.c
			if [ ! -f acgram.h ]; then \
			    $(YACC_CMD) $A/acgram.y; \
			    mv y.tab.h acgram.h; \
			    rm y.tab.c; \
			    touch acgram.h.ok; \
			fi

acgram.$o:		acgram.c $(P1_H)
			$(CC_CMD) $(YYDEBUG) acgram.c

aclex.c:		$A/aclex.l
			$(LEX) $(LFLAGS) $A/aclex.l
			mv lex.yy.c aclex.c

aclex.$o:		aclex.c acgram.h $(P1_H)
			$(CC_CMD) aclex.c

ansisup.$o:		$A/ansisup.c $(P1_H)
			$(CC_CMD) $A/ansisup.c

cgstuff.$o:		$A/cgstuff.c $(P1_H) $(MFILE2)
			$(CC_CMD) $A/cgstuff.c

debug.$o:		$A/debug.c \
				$(COMINC)/syms.h $(COMINC)/storclass.h $(P1_H)
			$(CC_CMD) -I$(COMINC) $A/debug.c

decl.$o:		$A/decl.c $(P1_H)
			$(CC_CMD) $A/decl.c

elfdebug.$o:		$A/elfdebug.c $(COMINC)/dwarf.h $(P1_H)
			$(CC_CMD) -I$(COMINC) $A/elfdebug.c

err.$o:			$A/err.c $(P1_H) $(CPP_INTERFACE)
			$(CC_CMD) $(CPP_INC) $A/err.c

init.$o:		$A/init.c $(P1_H)
			$(CC_CMD) $A/init.c

lex.$o:			$A/lex.c $(P1_H) acgram.h $(CPP_INTERFACE)
			$(CC_CMD) $(CPP_INC) $A/lex.c

lexsup.$o:		$A/lexsup.c acgram.h $(P1_H)
			$(CC_CMD) $(CPP_INC) $A/lexsup.c

main.$o:		$A/main.c $(P1_H) $(MDPINC)/sgs.h $(CPP_INTERFACE)
			$(CC_CMD) $(YYDEBUG) $(CPP_INC) $A/main.c

optim.$o:		$A/optim.c $(P1_H)
			$(CC_CMD) $A/optim.c

p1allo.$o:		$A/p1allo.c $(P1_H)
			$(CC_CMD) $A/p1allo.c

sharp.$o:		$A/sharp.c $(P1_H) acgram.h $(CPP_INTERFACE)
			$(CC_CMD) $(CPP_INC) $A/sharp.c

stmt.$o:		$A/stmt.c $(P1_H)
			$(CC_CMD) $A/stmt.c

sym.$o:			$A/sym.c $(P1_H)
			$(CC_CMD) $A/sym.c

trees.$o:		$A/trees.c $(P1_H)
			$(CC_CMD) $A/trees.c

types.$o:		$A/types.c $(P1_H)
			$(CC_CMD) $A/types.c

# install done by machine-dependent makefile

clean:
		-rm -f $(OF1:$o=o) $(OF2:$o=o) $(OF3:$o=o) $(ACLEX_O:$o=o)
		-rm -f $(OF1:$o=ln) $(OF2:$o=ln) $(OF3:$o=ln) $(ACLEX_O:$o=ln)
		-rm -f aclex.o lex.o
		-rm -f lint.out
		-rm -f acomp.ln


clobber:	clean
		-rm -f acomp acgram.[ch] acgram.h.ok aclex.c acomp.o

# lint pass2 stuff
lintp2:		$(OFILES)
# CC is assumed to be "lint" here, because of recursive make
		$(CC) $(OFILES) $(LIBS)
