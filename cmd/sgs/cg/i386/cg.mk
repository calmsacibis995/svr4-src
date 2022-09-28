#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cg:i386/cg.mk	1.3"
#
#cg.mk: local makefile for the i386 instance of cg
#
OBJECTS=	allo.$o cgen.$o comm2.$o cost.$o match.$o \
		reader.$o xdefs.$o local.$o local2.$o inline.$o \
		table.$o nail.$o stasg.$o fparith.$o atof2.$o picode.$o
HOSTCC=cc
LINT= lint
o=	o
INC= $(ROOT)/usr/include
# Enhanced asm's disabled by default
# Enable with INLINE=-DIN_LINE
INLINE=
INCPATH= -I. -I$(LOCAL) -I$(COMMON)
# NODBG= -DNODBG	to suppress debug code
# NODBG=		to include debug code
NODBG=
# FP emulator for Amdahl disabled by default.
# Enable with FP_EMULATE=-DFP_EMULATE
FP_EMULATE=
# Volatile support is for ANSI C type "volatile"
# Enable with VOL_SUPPORT= -DVOL_SUPPORT
VOL_SUPPORT=
# For read-only data section
# Enable with RODATA= -DRODATA
RODATA=
DEFLIST= -Di386 -DSTINCC -DCG -DFLEXNAMES $(OPTIM_SUPPORT)\
		$(INLINE) $(FP_EMULATE) $(NODBG) $(VOL_SUPPORT) $(RODATA)
COMMON=../common
LOCAL=.
COMSRC= $(COMMON)/allo.c $(COMMON)/atof2.c $(COMMON)/cgen.c \
	$(COMMON)/comm2.c $(COMMON)/cost.c $(COMMON)/fparith.c \
	$(COMMON)/inline.c $(COMMON)/match.c $(COMMON)/nail.c   \
	$(COMMON)/reader.c $(COMMON)/xdefs.c
LOCSRC= $(LOCAL)/local.c $(LOCAL)/local2.c $(LOCAL)/stasg.c $(LOCAL)/picode.c
SRC=$(COMSRC) $(LOCSRC)

INCLUDES= $(COMMON)/mfile1.h $(COMMON)/mfile2.h \
		$(COMMON)/manifest.h ./macdefs.h
CPRS=cprs
CC_CMD=	$(CC) -c $(CFLAGS) $(DEFLIST) $(INCPATH)

cg.o:	$(OBJECTS)
	$(LD) -r -o cg.o $(OBJECTS)

allo.o:	$(COMMON)/allo.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/allo.c

cgen.o:	$(COMMON)/cgen.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/cgen.c

comm2.o:	$(COMMON)/comm2.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/comm2.c

cost.o:	$(COMMON)/cost.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/cost.c

match.o:	$(COMMON)/match.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/match.c

reader.o:	$(COMMON)/reader.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/reader.c

nail.o:		$(COMMON)/nail.c $(INCLUDES) $(COMMON)/dope.h
	$(CC_CMD) $(COMMON)/nail.c

xdefs.o:	$(COMMON)/xdefs.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/xdefs.c

local.o:	$(LOCAL)/local.c $(INCLUDES)
	$(CC_CMD) $(LOCAL)/local.c

local2.o:	$(LOCAL)/local2.c $(INCLUDES)
	$(CC_CMD) $(LOCAL)/local2.c

stasg.o:	$(LOCAL)/stasg.c $(INCLUDES)
	$(CC_CMD) $(LOCAL)/stasg.c

picode.o:	$(LOCAL)/picode.c $(INCLUDES)
	$(CC_CMD) $(LOCAL)/picode.c

table.o:	table.c $(INCLUDES)
	$(CC_CMD) table.c

table.c:	stin sty
		./sty < stin > table.c

sty:	$(COMMON)/sty.y $(INCLUDES) $(COMMON)/dope.h
	$(YACC) $(YACCFLAGS) $(COMMON)/sty.y
	$(HOSTCC) $(DEFLIST) $(INCPATH) -o sty y.tab.c -ly

inline.o:	$(COMMON)/inline.c $(INCLUDES)
	$(CC_CMD) $(COMMON)/inline.c

fparith.o:	$(COMMON)/fparith.c $(COMMON)/fp.h
	$(CC_CMD) $(COMMON)/fparith.c

atof2.o:	$(COMMON)/atof2.c
	$(CC_CMD) $(COMMON)/atof2.c

clean:
	/bin/rm -f $(OBJECTS) table.c sty y.tab.c
	/bin/rm -f *.ln

clobber:	clean
	/bin/rm -f core make.out cg.o

lintit: $(SRC) table.c
	$(LINT) $(INCPATH) $(DEFLIST) -DNODBG \
	$(SRC) table.c >lint.out  2>&1

cg.ln:	$(SRC) table.c
	rm -f $(OBJECTS:$o=ln)
	$(LINT) -c $(INCPATH) $(DEFLIST) $(SRC) table.c
	cat $(OBJECTS:$o=ln) >cg.ln
