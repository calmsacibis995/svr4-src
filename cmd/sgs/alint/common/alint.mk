#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)alint:common/alint.mk	1.6.1.5"

# Required macros

ROOT=
SGS=
OWN=
GRP=
CC=		cc
CFLAGS=		-O
LDLIBS=
LINT=		lint
LINTFLAGS=
SGSBASE=	../..
INS=		$(SGSBASE)/sgs.install
STRIP=		strip

ACCCOMMON=
A=		$(ACCCOMMON)
ACC_MDP=
ACOMP_O= 
CG_MDP=
CG_COMMON=
DEFLIST= 	-DLINT
IN_LINE=
FP_EMULATE=
OPTIM_SUPPORT=
NODBG=
ACC_INC=
INCLIST= 	$(ACC_INC)
OPTIONS= 	$(DEFLIST) $(IN_LINE) $(FP_EMULATE) $(OPTIM_SUPPORT) $(NODBG)
LIBS=		-ll
L_COM=		$(SGSBASE)/alint/common
L=		$(L_COM)
LINK_MODE=
CC_CMD=		$(CC)  $(CFLAGS) $(NODBG) $(INCLIST) $(OPTIONS)
o=o

P1_H=	$A/p1.h $A/aclex.h $A/ansisup.h $A/cgstuff.h \
	$A/decl.h $A/err.h $A/host.h $A/init.h \
	$A/node.h $A/optim.h $A/stmt.h $A/sym.h \
	$A/target.h $A/tblmgr.h $A/trees.h $A/types.h \
	$L/lint.h  $L/xl_byte.h \
	$(ACC_MDP)/mddefs.h $(CG_MDP)/macdefs.h $(CG_COMMON)/manifest.h 

L_H=	$L/msgbuf.h $L/lnstuff.h $L/lpass2.h $L/xl_byte.h

L2_H=	$L/tables2.h $L/lpass2.h $L/xl_byte.h

HFILES=	$L/lint.h $L/lpass2.h $L/tables2.h $L/ldope.h \
	$L/lnstuff.h $L/msgbuf.h $L/xl_byte.h

SOURCES1=	$L/lint.c $L/messages.c $L/msgbuf.c $L/hooks.c $L/lnstuff.c \
		$L/ldope.c $L/directives.c $L/lmain.c $L/postopt.c $L/cxref.c \
		$L/xl_byte.c

SOURCES2=	$L/lpass2.c $L/messages.c $L/msgbuf.c $L/formchk.c \
		$L/tables2.c $L/debug2.c $L/xl_byte.c

OBJECTS1=	lint.$o messages1.$o msgbuf1.$o hooks.$o lnstuff.$o \
		ldope.$o directives.$o lmain.$o postopt.$o cxref.$o \
		xl_byte.$o
OBJECTS2=	lpass2.$o messages2.$o msgbuf2.$o formchk.$o \
		tables2.$o debug2.$o xl_byte.$o

all build:		lint1 lint2

lint1:			$(OBJECTS1) $(ACOMP_O)
			$(CC) $(LINK_MODE) -o lint1 $(CFLAGS) $(OBJECTS1) $(ACOMP_O) $(LIBS)

lint2:			$(OBJECTS2) $(P1_H)
			$(CC) $(LINK_MODE) -o lint2 $(CFLAGS) $(OBJECTS2)

lint.$o:		$L/lint.c $(P1_H) $L/lnstuff.h $L/ldope.h
			$(CC_CMD) -c $L/lint.c

ldope.$o:		$L/ldope.c $(P1_H) $L/ldope.h
			$(CC_CMD) -c $L/ldope.c

directives.$o:		$L/directives.c $(P1_H)
			$(CC_CMD) -c $L/directives.c

postopt.$o:		$L/postopt.c $(P1_H) $L/lnstuff.h $L/ldope.h
			$(CC_CMD) -c $L/postopt.c

lmain.$o:		$L/lmain.c $(P1_H) $L/lnstuff.h $L/ldope.h 
			$(CC_CMD) -c $L/lmain.c

messages1.$o:		$L/messages.c $(L_H)
			$(CC_CMD) -c $L/messages.c
			mv messages.$o messages1.$o

msgbuf1.$o:		$L/msgbuf.c $L/msgbuf.h
			$(CC_CMD) -c $L/msgbuf.c
			mv msgbuf.$o msgbuf1.$o

messages2.$o:		$L/messages.c $(L_H)
			$(CC_CMD) -c -DLINT2 $L/messages.c
			mv messages.$o messages2.$o

msgbuf2.$o:		$L/msgbuf.c $L/msgbuf.h
			$(CC_CMD) -c -DLINT2 $L/msgbuf.c
			mv msgbuf.$o msgbuf2.$o

lpass2.$o:		$L/lpass2.c $(P1_H) $L/lnstuff.h $(L2_H)
			$(CC_CMD) -c -DLINT2 $L/lpass2.c

formchk.$o:		$L/formchk.c $(P1_H) $L/lnstuff.h $L/lpass2.h
			$(CC_CMD) -c -DLINT2 $L/formchk.c

tables2.$o:		$L/tables2.c $(P1_H) $L/lnstuff.h $(L2_H)
			$(CC_CMD) -c -DLINT2 $L/tables2.c

debug2.$o:		$L/debug2.c $L/lnstuff.h $(L2_H) $(P1_H)
			$(CC_CMD) -c -DLINT2 $L/debug2.c

hooks.$o:		$L/hooks.c $(P1_H)
			$(CC_CMD) -c $L/hooks.c

lnstuff.$o:		$L/lnstuff.c $(P1_H) $L/lnstuff.h
			$(CC_CMD) -c $L/lnstuff.c

xl_byte.$o:		$L/xl_byte.c $(P1_H) $(L_H)
			$(CC_CMD) -c $L/xl_byte.c

cxref.$o:		$L/cxref.c $(P1_H)
			$(CC_CMD) -c $L/cxref.c

lintit:		llint1 llint2

llint1:		$(OBJECTS1)
		$(CC) $(OBJECTS1) others.ln $(LIBS)

llint2:		$(OBJECTS2) $(P1_H)
		$(CC) $(CFLAGS) $(OBJECTS2)

clean:
		rm -f $(OBJECTS1) $(OBJECTS2)
