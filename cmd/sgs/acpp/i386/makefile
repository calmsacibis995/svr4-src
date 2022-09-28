#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)acpp:i386/Makefile	1.14"
#	acpp Makefile
#
#
SGS	= i386
#
#	External Directories
#
ROOT	=
SGSBASE	= ../..
CCSLIB	= $(ROOT)/usr/ccs/lib
INCDIR	= $(SGSBASE)/inc
INC	= $(ROOT)/usr/include
MDPINC	= $(SGSBASE)/inc/i386
INSDIR	= $(LIBDIR)
#
#	Internal Directories
#
CPPCOM	= $(SGSBASE)/acpp/common
PROFDIR=        ./profdir
PROFOUT=        ./profdir
#
#	Compilation Parameters
#
CC	= cc
CFLAGS	= -O
NODBG	= 
DEBUG	= 
CXREF	=
CPLUSPLUS = 
DEFLIST	= $(CXREF) $(DEBUG) $(NODBG) $(CPLUSPLUS) -DTRANSITION -DC_SIGNED_RS -DFILTER -DC_CHSIGN -DRTOLBYTES
INCLIST = -I$(CPPCOM) -I$(MDPINC)
#
# DFLTINC is the default ./usr/include directory
#
DFLTINC = /usr/include
#
# SGSINC not used
# ENVPARMS = -DSGSINC=\"$(INCDIR)\" -DINC=\"$(INC)\"
#
ENVPARMS = -DINC=\"$(INC)\" -DDFLTINC=\"$(DFLTINC)\"
PREASPREDS= -DPREASPREDS="\"machine(i386)\",\"system(unix)\",\"cpu(i386)\""
PREDMACROS= -DPREDMACROS="\"i386\",\"unix\""
ENV	=
LDFLAGS	=
LDLIBS =
LIBES	=
LINK_MODE =
CC_CMD	= $(CC) $(CFLAGS) $(INCLIST) $(DEFLIST) $(ENVPARMS) $(ENV) -c 
#
#	Lint Parameters
#
LINT	= lint
LINTFLAGS = 
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
PROFLAGS=       -g
LP	= lp
INS	= $(SGSBASE)/sgs.install
#
PROT	= 775
GRP	= bin
OWN	= bin
#
FRC	=
#
#	Files making up acpp
#
OBJECTS	= buf.$O chartbl.$O expand.$O file.$O group.$O lex.$O \
	  main.$O  predicate.$O syms.$O token.$O xpr.$O

SOURCES = $(CPPCOM)/buf.c $(CPPCOM)/chartbl.c $(CPPCOM)/expand.c $(CPPCOM)/file.c \
        $(CPPCOM)/group.c $(CPPCOM)/lex.c $(CPPCOM)/main.c $(CPPCOM)/predicate.c \
        $(CPPCOM)/syms.c $(CPPCOM)/token.c $(CPPCOM)/xpr.c

HFILES	= $(CPPCOM)/interface.h $(CPPCOM)/syms.h $(CPPCOM)/cpp.h $(CPPCOM)/group.h \
	$(CPPCOM)/predicate.h 

all:	acpp

acpp:	$(OBJECTS) $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LINK_MODE) -o acpp $(OBJECTS) $(LIBES) $(LDLIBS)

buf.$O:	$(CPPCOM)/buf.c $(CPPCOM)/buf.h $(CPPCOM)/file.h $(CPPCOM)/group.h \
	$(CPPCOM)/syms.h $(CPPCOM)/predicate.h $(CPPCOM)/cpp.h \
	$(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/buf.c

chartbl.$O: $(CPPCOM)/chartbl.c $(CPPCOM)/cpp.h
	$(CC_CMD) $(CPPCOM)/chartbl.c

expand.$O: $(CPPCOM)/expand.c $(CPPCOM)/buf.h $(CPPCOM)/syms.h \
	$(CPPCOM)/predicate.h $(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/expand.c

file.$O: $(CPPCOM)/file.c $(CPPCOM)/buf.h $(CPPCOM)/file.h \
	$(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/file.c

group.$O: $(CPPCOM)/group.c $(CPPCOM)/file.h $(CPPCOM)/group.h\
	$(CPPCOM)/syms.h $(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/group.c

lex.$O: $(CPPCOM)/lex.c $(CPPCOM)/buf.h $(CPPCOM)/syms.h \
	$(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/lex.c

main.$O: $(CPPCOM)/main.c $(CPPCOM)/buf.h $(CPPCOM)/file.h $(CPPCOM)/group.h\
	$(CPPCOM)/syms.h $(CPPCOM)/predicate.h $(CPPCOM)/cpp.h \
	$(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/main.c

predicate.$O: $(CPPCOM)/predicate.c $(CPPCOM)/predicate.h $(CPPCOM)/cpp.h \
	$(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(PREASPREDS) $(CPPCOM)/predicate.c

syms.$O: $(CPPCOM)/syms.c $(CPPCOM)/buf.h $(CPPCOM)/file.h $(CPPCOM)/syms.h \
	$(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(PREDMACROS) $(CPPCOM)/syms.c

token.$O: $(CPPCOM)/token.c $(CPPCOM)/buf.h $(CPPCOM)/cpp.h $(CPPCOM)/interface.h \
	$(FRC)
	$(CC_CMD) $(CPPCOM)/token.c

xpr.$O: $(CPPCOM)/xpr.c $(CPPCOM)/syms.h $(CPPCOM)/cpp.h $(CPPCOM)/interface.h $(FRC)
	$(CC_CMD) $(CPPCOM)/xpr.c

shrink:	clobber
#
clean :
	$(RM) -f $(OBJECTS)
#
clobber: clean
	$(RM) -f acpp
#
install : $(INSDIR)/$(SGS)acpp
#
$(INSDIR)/$(SGS)acpp:	acpp
	$(CP) acpp acpp.bak
	$(STRIP) acpp
	sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)acpp acpp
	mv acpp.bak acpp
#
save:
	$(RM) -f $(INSDIR)/$(SGS)acpp.back
	$(CP) $(INSDIR)/$(SGS)acpp $(INSDIR)/$(SGS)acpp.back
#
uninstall:	$(INSDIR)/$(SGS)acpp.back
	$(RM) -f $(INSDIR)/$(SGS)acpp
	$(CP) $(INSDIR)/$(SGS)acpp.back $(INSDIR)/$(SGS)acpp
#
lint:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(SOURCES)
#
listing:	$(SOURCES) 
	$(PRINT) $(PRFLAGS) $(SOURCES) | $(LP)
#
FRC:
