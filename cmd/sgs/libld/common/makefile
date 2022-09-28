#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libld:common/makefile	1.15"
#
#	COMMON LIBLD MAKEFILE
#
#
SGS	= 
#
#	Host Dependent Parameters
#	ARFORMAT = archive format
#		OLDAR	= old format
#		PORT5AR	= 5.0 random access format
#		PORTAR	= 6.0 portable ascii format
#
FLEX	= -DFLEXNAMES
ARFORMAT = PORTAR
#
#	Compilation Parameters
#
CC	= cc
CFLAGS	= -O
DEFLIST	= -D$(ARFORMAT) $(FLEX)
INCLIST	= -I$(LIBLDCOM) -I$(INCCOM)
ENV	= 
FRC	=
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
AR	= ar
ARFLAGS	= r
AR_CMD	= $(AR) $(ARFLAGS)
#
LINT	= lint
LINTFLAGS =
LINT_CMD = $(LINT) $(LINTFLAGS)
O	= o
#
#	External Directories
#
ROOT 	= 
LIBDIR	= $(ROOT)/lib
ULIBDIR	= $(ROOT)/usr/lib
#
#	Internal Directories
#
SGSBASE	=  ../..
INCCOM	= $(SGSBASE)/inc/common
LIBLDCOM = $(SGSBASE)/libld/common
#
RM	= rm
PRINT	= pr
PRFLAGS	= -n
LP	= lp
LPFLAGS	=
#
OWN	= bin
GRP	= bin
#
OBJECTS = ldaclose.$O ldaopen.$O ldclose.$O ldahread.$O ldlseek.$O ldnlseek.$O \
	ldnrseek.$O ldnsseek.$O ldnshread.$O ldopen.$O ldrseek.$O ldlread.$O \
	ldsseek.$O ldshread.$O ldfhread.$O ldtbseek.$O ldohseek.$O ldtbread.$O \
	ldtbindex.$O ldgetname.$O allocldptr.$O freeldptr.$O vldldptr.$O \
	sgetl.$O sputl.$O
#
CFILES = $(LIBLDCOM)/ldaclose.c $(LIBLDCOM)/ldaopen.c $(LIBLDCOM)/ldclose.c \
	$(LIBLDCOM)/ldahread.c $(LIBLDCOM)/ldlseek.c $(LIBLDCOM)/ldnlseek.c \
	$(LIBLDCOM)/ldnrseek.c $(LIBLDCOM)/ldnsseek.c $(LIBLDCOM)/ldnshread.c \
	$(LIBLDCOM)/ldopen.c $(LIBLDCOM)/ldrseek.c $(LIBLDCOM)/ldlread.c \
	$(LIBLDCOM)/ldsseek.c $(LIBLDCOM)/ldshread.c $(LIBLDCOM)/ldfhread.c \
	$(LIBLDCOM)/ldtbseek.c $(LIBLDCOM)/ldohseek.c $(LIBLDCOM)/ldtbread.c \
	$(LIBLDCOM)/ldtbindex.c $(LIBLDCOM)/allocldptr.c \
	$(LIBLDCOM)/freeldptr.c $(LIBLDCOM)/vldldptr.c $(LIBLDCOM)/sgetl.c \
	$(LIBLDCOM)/sputl.c $(LIBLDCOM)/ldgetname.c
#
HEADERS	= $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/scnhdr.h \
	  $(INCCOM)/syms.h $(INCCOM)/storclass.h $(INCCOM)/linenum.h \
	  $(LIBLDCOM)/lddef.h
#
build:	all
#
all:	libld.a
#
libld.a: $(OBJECTS) $(FRC)
	$(RM) -f libld.a
	$(AR_CMD) libld.a $(OBJECTS)
#
allocldptr.$O:	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/lddef.h \
		$(LIBLDCOM)/allocldptr.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/allocldptr.c
#
freeldptr.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/lddef.h \
	      $(LIBLDCOM)/freeldptr.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/freeldptr.c
#
vldldptr.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/lddef.h \
	     $(LIBLDCOM)/vldldptr.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/vldldptr.c
#
ldaclose.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldaclose.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldaclose.c
#
ldaopen.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldaopen.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldaopen.c
#
ldclose.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldclose.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldclose.c
#
ldahread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldahread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldahread.c
#
ldlseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
		$(LIBLDCOM)/ldlseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldlseek.c
#
ldnlseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     $(LIBLDCOM)/ldnlseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldnlseek.c
#
ldnrseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     $(LIBLDCOM)/ldnrseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldnrseek.c
#
ldnsseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     $(LIBLDCOM)/ldnsseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldnsseek.c
#
ldnshread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	      $(LIBLDCOM)/ldnshread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldnshread.c
#
ldopen.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldopen.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldopen.c
#
ldrseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
		$(INCCOM)/ldfcn.h $(LIBLDCOM)/ldrseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldrseek.c
#
ldtbread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/syms.h $(INCCOM)/storclass.h \
	     $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldtbread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldtbread.c
#
ldsseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
		$(INCCOM)/ldfcn.h $(LIBLDCOM)/ldsseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldsseek.c
#
ldshread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     $(LIBLDCOM)/ldshread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldshread.c
#
ldfhread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(LIBLDCOM)/ldfhread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldfhread.c
#
ldtbseek.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(LIBLDCOM)/ldtbseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldtbseek.c
#
ldohseek.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(LIBLDCOM)/ldohseek.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldohseek.c
#
ldtbindex.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	      $(INCCOM)/storclass.h $(LIBLDCOM)/ldtbindex.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldtbindex.c
#
ldlread.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h \
	    $(INCCOM)/scnhdr.h $(INCCOM)/syms.h \
	    $(INCCOM)/linenum.h $(INCCOM)/storclass.h \
	    $(LIBLDCOM)/ldlread.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldlread.c
#
sgetl.$O:  $(LIBLDCOM)/sgetl.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/sgetl.c
#
sputl.$O:  $(LIBLDCOM)/sputl.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/sputl.c
#
ldgetname.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	      $(INCCOM)/storclass.h $(LIBLDCOM)/ldgetname.c $(FRC)
	$(CC_CMD) $(LIBLDCOM)/ldgetname.c
#
install:	$(LIBDIR)/libld.a
#
$(LIBDIR)/libld.a:	libld.a
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(LIBDIR)/libld.a libld.a
#
shrink:
	$(RM) -f $(OBJECTS) libld.a
#
clean:
	$(RM) -f $(OBJECTS)
#
clobber:	clean
	$(RM) -f libld.a llib-lld.ln
#
lint:	$(HEADERS) $(CFILES)
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES)
#
llib-lld.ln:	$(LIBLDCOM)/llib-lld.c
	$(LINT) $(LIBLDCOM)/llib-lld.c -c
#
install.ln:	llib-lld.ln
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(ULIBDIR)/llib-lld.ln  llib-lld.ln
#
listing:	$(HEADERS) $(CFILES)
	$(PRINT) $(PRFLAGS) $(HEADERS) $(SOURCE) | $(LP) $(LPFLAGS)
#
FRC:
