#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libelf:common/Makefile	1.7"

COM=		../common
COFF=		../coff
MISC=		../misc
MMAP=
DEFLIST=	-D$(ARFORMAT)
DEFCOM=		$(MMAP)
DEFCOFF=
DEFMISC=	-D$(MACHNM) -DELF
PRODUCTS=	libelf.a

AR=		ar
ARFLAGS=	r
AR_CMD=		$(AR) $(ARFLAGS)
CC_CMD=		$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
ARFORMAT=	PORTAR
CFLAGS=		-O
GRP=		bin
LDLIBS=
LORDER=		lorder
OWN=		bin
ROOT=
SGS=

SGSBASE=	../..
CCSBIN=		$(ROOT)/usr/ccs/bin
CCSLIB=		$(ROOT)/usr/ccs/lib
COMINC=		$(SGSBASE)/inc/common
SGSINC=		$(SGSBASE)/inc/$(MACH)
ENV=
INC=
INCLIST=	-I$(COM) -I$(COMINC)
INCSYS=
INS=		$(SGSBASE)/sgs.install

LINT=		lint
LINTFLAGS=

COFFHDR=\
	$(COFF)/coff.h\
	$(COFF)/cofftab.h

COFFSRC=\
	$(COFF)/coff.c\
	$(COFF)/coff386.c\
	$(COFF)/coffm32.c\
	$(COFF)/coffscn.c\
	$(COFF)/coffshdr.c\
	$(COFF)/coffstr.c\
	$(COFF)/coffsym.c\
	$(COFF)/cofftab.c

COFFOBJ=\
	coff.o\
	coff386.o\
	coffm32.o\
	coffscn.o\
	coffshdr.o\
	coffstr.o\
	coffsym.o\
	cofftab.o

COMHDR=\
	$(COM)/decl.h\
	$(COM)/error.h\
	$(COM)/foreign.h\
	$(COM)/member.h\
	$(COM)/syn.h\
	$(COM)/tune.h

COMSRC=\
	$(COM)/ar.c\
	$(COM)/begin.c \
	$(COM)/cntl.c\
	$(COM)/cook.c\
	$(COM)/data.c\
	$(COM)/end.c \
	$(COM)/error.c \
	$(COM)/fill.c\
	$(COM)/flag.c \
	$(COM)/foreign.c\
	$(COM)/getarhdr.c \
	$(COM)/getarsym.c\
	$(COM)/getbase.c \
	$(COM)/getdata.c\
	$(COM)/getehdr.c\
	$(COM)/getident.c\
	$(COM)/getphdr.c\
	$(COM)/getscn.c \
	$(COM)/getshdr.c\
	$(COM)/hash.c\
	$(COM)/input.c\
	$(COM)/kind.c\
	$(COM)/ndxscn.c\
	$(COM)/newdata.c\
	$(COM)/newehdr.c\
	$(COM)/newphdr.c\
	$(COM)/newscn.c\
	$(COM)/next.c\
	$(COM)/nextscn.c\
	$(COM)/output.c\
	$(COM)/rand.c\
	$(COM)/rawdata.c\
	$(COM)/rawfile.c\
	$(COM)/rawput.c\
	$(COM)/strptr.c\
	$(COM)/svr4.c\
	$(COM)/update.c\
	$(COM)/xlate.c

COMOBJ=\
	ar.o\
	begin.o\
	cntl.o\
	cook.o\
	data.o\
	end.o\
	error.o\
	fill.o\
	flag.o\
	foreign.o\
	getarhdr.o\
	getarsym.o\
	getbase.o\
	getdata.o\
	getehdr.o\
	getident.o\
	getphdr.o\
	getscn.o\
	getshdr.o\
	hash.o\
	input.o\
	kind.o\
	ndxscn.o\
	newdata.o\
	newehdr.o\
	newphdr.o\
	newscn.o\
	next.o\
	nextscn.o\
	output.o\
	rand.o\
	rawdata.o\
	rawfile.o\
	rawput.o\
	strptr.o\
	svr4.o\
	update.o\
	xlate.o

MISHDR=\
	$(MISC)/String.h\
	$(MISC)/elf_dem.h

MISCSRC=\
	$(MISC)/String.c\
	$(MISC)/args.c\
	$(MISC)/demangle.c\
	$(MISC)/nlist.c\
	$(MISC)/nplist.c

MISCOBJ=\
	String.o\
	args.o\
	demangle.o\
	nlist.o\
	nplist.o


HDR=	$(COFFHDR) $(COMHDR) $(MISCHDR)
OBJ=	$(COFFOBJ) $(COMOBJ) $(MISCOBJ)
SRC=	$(COFFSRC) $(COMSRC) $(MISCSRC)

mmap:		$(COM)/mmap.c
	@ echo ======================================;\
	echo Checking system call availability.;\
	echo Test compilation failure is ok.;\
	echo $(CC) $(INCLIST) $(DEFLIST) -o mmap $(COM)/mmap.c;\
	if $(CC) $(INCLIST) $(DEFLIST) -o mmap $(COM)/mmap.c;\
	then	rm -f mmap;\
		echo Successful: enabling memory mapping;\
		echo ======================================;\
		$(MAKE) -e MMAP=-DMMAP_IS_AVAIL all;\
	else	rm -f mmap;\
		echo Unsuccessful: disabling memory mapping;\
		echo ======================================;\
		$(MAKE) -e MMAP= all;\
	fi;\
	rm -f mmap

all:		$(PRODUCTS)

libelf.a:	$(OBJ)
	$(AR_CMD) libelf.a `$(LORDER) $(OBJ) | tsort`

$(COFFOBJ):	$(COFF)/coff.h\
		$(COM)/decl.h\
		$(COM)/error.h\
		$(COM)/syn.h
	$(CC_CMD) $(DEFCOFF) $(COFF)/$(@F:.o=.c)

$(COMOBJ):	$(COM)/decl.h\
		$(COM)/error.h\
		$(COM)/syn.h
	$(CC_CMD) $(DEFCOM) $(COM)/$(@F:.o=.c)

$(MISCOBJ):	$(MISC)/elf_dem.h\
		$(MISC)/String.h
	$(CC_CMD) $(DEFMISC) $(MISC)/$(@F:.o=.c)

clean:
	-rm -f $(OBJ) $(COM)/error.c $(COM)/error.h $(COM)/xlate.c

clobber:	clean
	-rm -f libelf.a
	-rm -f llib-lelf.ln

install:	all
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/libelf.a libelf.a
	#/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/llib-lelf.ln llib-lelf.ln

lintit:	$(SRC) $(HDR)
	$(LINT)  $(INCLIST) $(DEFLIST) -u $(SRC) >lint.out

llib-lelf.ln:	$(COM)/llib-lelf.c
	$(LINT) -oelf $(INCLIST) $(COM)/llib-lelf.c

String.o:	$(MISC)/String.c
ar.o:		$(COM)/ar.c	$(COM)/member.h
args.o:		$(MISC)/args.c
begin.o:	$(COM)/begin.c	$(COM)/foreign.h $(COM)/member.h
coff.o:		$(COFF)/coff.c	$(COM)/foreign.h
coff386.o:	$(COFF)/coff386.c	$(COFF)/cofftab.h
coffm32.o:	$(COFF)/coffm32.c	$(COFF)/cofftab.h
coffscn.o:	$(COFF)/coffscn.c
coffshdr.o:	$(COFF)/coffshdr.c
coffstr.o:	$(COFF)/coffstr.c
coffsym.o:	$(COFF)/coffsym.c
cofftab.o:	$(COFF)/cofftab.c	$(COFF)/cofftab.h
cntl.o:		$(COM)/cntl.c
cook.o:		$(COM)/cook.c
data.o:		$(COM)/data.c
demangle.o:	$(MISC)/demangle.c
end.o:		$(COM)/end.c	$(COM)/member.h
error.o:	$(COM)/error.c
fill.o:		$(COM)/fill.c
flag.o:		$(COM)/flag.c
foreign.o:	$(COM)/foreign.c	$(COM)/foreign.h
getarhdr.o:	$(COM)/getarhdr.c	$(COM)/member.h
getarsym.o:	$(COM)/getarsym.c
getbase.o:	$(COM)/getbase.c
getdata.o:	$(COM)/getdata.c
getehdr.o:	$(COM)/getehdr.c
getident.o:	$(COM)/getident.c
getphdr.o:	$(COM)/getphdr.c
getscn.o:	$(COM)/getscn.c
getshdr.o:	$(COM)/getshdr.c
input.o:	$(COM)/input.c	$(COM)/tune.h
hash.o:		$(COM)/hash.c
kind.o:		$(COM)/kind.c
ndxscn.o:	$(COM)/ndxscn.c
newdata.o:	$(COM)/newdata.c
newehdr.o:	$(COM)/newehdr.c
newphdr.o:	$(COM)/newphdr.c
newscn.o:	$(COM)/newscn.c
next.o:		$(COM)/next.c
nextscn.o:	$(COM)/nextscn.c
nlist.o:	$(MISC)/nlist.c
nplist.o:	$(MISC)/nplist.c
output.o:	$(COM)/output.c
rand.o:		$(COM)/rand.c
rawdata.o:	$(COM)/rawdata.c
rawfile.o:	$(COM)/rawfile.c
rawput.o:	$(COM)/rawput.c
puthdr.o:	$(COM)/puthdr.c
strptr.o:	$(COM)/strptr.c
svr4.o:		$(COM)/svr4.c
update.o:	$(COM)/update.c
version.o:	$(COM)/version.c
xlate.o:	$(COM)/xlate.c

$(COM)/error.c:	$(COM)/error.list $(COM)/error.c.awk
	awk -f $(COM)/error.c.awk $(COM)/error.list >$(COM)/error.c

$(COM)/error.h:	$(COM)/error.list $(COM)/error.h.awk
	awk -f $(COM)/error.h.awk $(COM)/error.list >$(COM)/error.h

$(COM)/xlate.c:	$(COM)/xlate.m4
	m4 < $(COM)/xlate.m4 > $(COM)/xlate.c
