#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cpp:i386/makefile	1.1.2.5"
#	Cpp Makefile
#
#
SGS	= i386
BASE	= ../..
#
# the two variables NEW_MACH and NEW_SYS are used to create new instances
# of cpp by giving new machine and/or system names to cpp.c.  These
# should be used with PD_MACH=D_newmach and PD_SYS=D_newsys.
# NEW_MACH and NEW_SYS are used (just as SGSINC is) as a string in cpp.c
#
PD_MACH	= D_nomach
PD_SYS	= D_unix
# PRAGMA=		to disable #pragma recognition
# PRAGMA= -DPRAGMA	to enable
PRAGMA= -DPRAGMA
NEW_MACH =
NEW_SYS	=
#
#	External Directories
#
ROOT	=
SGSBASE	= ../..
CCSLIB	= $(ROOT)/usr/ccs/lib
INCDIR	= 
DFLTINC	= /usr/include
#
#	Internal Directories
#
CPPCOM	= $(SGSBASE)/cpp/common
#
#	Compilation Parameters
#
CC	= cc
CFLAGS	= -O
FLEX	= -DFLEXNAMES
DEFLIST	= $(FLEX) $(PRAGMA)
ENVPARMS = -DSGSINC=\"$(INCDIR)\" -DUSRINC=\"$(DFLTINC)\" -DPD_MACH=$(PD_MACH) -DPD_SYS=$(PD_SYS)
ENV	=
LINK_MODE=
LDFLAGS	= 
LDLIBS =
LIBES	=
#
#	Lint Parameters
#
LINT	= lint
LINTFLAGS = -p
O	= o
#
#	Other Commands
#
SH	= sh
YACC	= yacc
YFLAGS	= -d
STRIP	= strip
SPFLAGS	=
CP	= cp
RM	= rm
PRINT	= pr
PRFLAGS	= -n
LP	= lp
#
PROT	= 775
GRP	= bin
OWN	= bin
#
FRC	=
#
#	Files making up cpp
#
OBJECTS	= cpp.$O cpy.$O rodata.$O yylex.$O
SOURCE	= $(CPPCOM)/cpy.y $(CPPCOM)/yylex.c $(CPPCOM)/cpp.c
#
all:	cpp
#
cpp:	$(OBJECTS) $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LINK_MODE) -o cpp $(OBJECTS) $(LIBES) $(LDLIBS)
#
y.tab.h cpy.c : $(CPPCOM)/cpy.y $(FRC)
	$(YACC) $(YFLAGS) $(CPPCOM)/cpy.y
	-if pdp11 || vax; \
	then \
		$(SH) $(CPPCOM)/:yyfix >rodata.c; \
	else \
		>rodata.c; \
	fi
	mv y.tab.c cpy.c
#
cpy.$O:	cpy.c $(FRC)
	$(CC) $(CFLAGS) $(DEFLIST) $(ENV) -c cpy.c
#
yylex.$O: $(CPPCOM)/yylex.c y.tab.h $(FRC)
	$(CC) $(CFLAGS) $(DEFLIST) $(ENV) -I. -c $(CPPCOM)/yylex.c
#
cpp.$O: $(CPPCOM)/cpp.c $(FRC)
	$(CC) $(CFLAGS) $(DEFLIST) $(ENVPARMS) $(NEW_SYS) $(NEW_MACH) $(ENV) -c $(CPPCOM)/cpp.c
#
rodata.$O: cpy.c $(FRC)
	$(CC) $(CFLAGS) $(DEFLIST) $(ENV) -S rodata.c
	-if pdp11 || vax; \
	then \
		$(SH) $(CPPCOM)/:rofix rodata.s; \
	fi
	$(CC) $(CFLAGS) -c rodata.s
#
shrink:	clobber
#
clean :
	$(RM) -f cpy.c y.tab.h rodata.c rodata.s $(OBJECTS)
#
clobber: clean
	$(RM) -f cpp
#
install : $(CCSLIB)/$(SGS)cpp
#
$(CCSLIB)/$(SGS)cpp:	cpp
	$(CP) cpp cpp.bak
	$(STRIP) cpp
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(CCSLIB)/$(SGS)cpp cpp
	mv cpp.bak cpp
#
save:
	$(RM) -f $(CCSLIB)/$(SGS)cpp.back
	$(CP) $(CCSLIB)/$(SGS)cpp $(CCSLIB)/$(SGS)cpp.back
#
uninstall:	$(CCSLIB)/$(SGS)cpp.back
	$(RM) -f $(CCSLIB)/$(SGS)cpp
	$(CP) $(CCSLIB)/$(SGS)cpp.back $(CCSLIB)/$(SGS)cpp
#
lint:	$(SOURCE)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(SOURCE)
#
listing:	$(SOURCE) 
	$(PRINT) $(PRFLAGS) $(SOURCE) $(CPPCOM)/:yyfix $(CPPCOM)/:rofix | $(LP)
#
FRC:
