#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xenv:i386/template.mk	1.1.6.1"
#
#	Intel 80386 GLOBAL MAKEFILE
#
#
#	SGS indicates the prefix to be applied to the installed commands.
SGS=i386
#
#	ARCH indicates the architecture of the host machine
#		AR16WR=11/70, AR32WR=vax, AR32W=ibm, 3b20, etc
ARCH=AR32W
#
#	The following macros define the various installation directories.
#	Note that if relative pathnames are used, they must allow for the
#	"cd" issued prior to each make.
#
#	SGSBASE is the directory under which all the sgs source lives
SGSBASE= ../..
#
#	CCSBIN is the directory for installing executable ("bin") files.
#	It should correspond to the same directory as specified in AS and LD
#	in paths.h.
CCSBIN=
#
#	CCSLIB is the directory for installing libraries and executable
#	files not intended for direct user execution (e.g., assembler pass1).
#	It should correspond to the same directory as specified in COMP,
#	OPTIM, AS1, and AS2 in paths.h.
CCSLIB=
ETC=
#
#	Specify the byte order for this SGS instance.
#		FBO = Forward Byte Order (3B20, IBM)
#		RBO = Reverse Byte Order (DEC)
DBO=RBO
#
#	If host machine is running a pre-5.0 release of UNIX
#	then set Archive Format to "OLDAR".
#	Starting with the 5.0 UNIX (SVR1)  release the Archive Format
#	should be set to "PORTAR".
#	If building a 5.0 release on a host that is running
#	System V Rel. 2.0 (SVR2), then set ARFORMAT to PORT5AR.
#
ARFORMAT=PORTAR
#
#	Starting with the SVR2 release of UNIX,
#	if flexnames feature is desired, then set FLEX
#	to -DFLEXNAMES.
#	If host machine is running a pre-SVR2 release of UNIX
#	then set FLEX to null (ie. FLEX= ).
#
FLEX=-DFLEXNAMES
#	This is the machine ID field. The valid values are
#	u3b15, u3b5, u3b2, or i386.
MACH=i386
#
#	The following parameter specifies the default include directory
#	for cpp. If not defined the value will be ROOT/usr/include.
#
LPASS= .
NATIVE=
INC=
INCSYS=
DFLTINC=
NOUSRINC=NOUSRINC
#
#
OWN=
GRP=
#
#
MAKE=make
YACC=yacc
LEX=lex
CC=cc
AR=ar
CFLAGS=-c
FFLAG=
ENV=
ROOT=
VERSION=
MACHINC=$(SGSBASE)/inc/i386
LIBLD=$(SGSBASE)/libld/i386/libld.a
LIBELF=$(SGSBASE)/libelf/i386/libelf.a
#
#
# The CCS by default produces dynamically-linked executables.
# By setting LINK_MODE to -dn, dynamic linking is turned off.
LINK_MODE=
#
ENVPARMS=MAKE="$(MAKE)" SGS="$(SGS)" ARCH="$(ARCH)" OWN="$(OWN)" GRP="$(GRP)" \
	DBO="$(DBO)" ARFORMAT="$(ARFORMAT)" FLEX="$(FLEX)" \
	LIBLD="$(LIBLD)" LIBELF="$(LIBELF)" MACHINC="$(MACHINC)" \
	YACC="$(YACC)" LEX="$(LEX)" MACH="$(MACH)" NATIVE="$(NATIVE)"

CPPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" INC="$(INC)" DFLTINC="$(DFLTINC)"

CCPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" USRINC="$(NOUSRINC)" INC="$(NOUSRINC)"

DIRPARMS=CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" ETC="$(ETC)"
#
#
all:	tools libs
	cd $(SGSBASE)/ar/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(SGSBASE)/acpp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) clean
	cd $(SGSBASE)/acomp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) 
	cd $(SGSBASE)/acpp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) clean
	cd $(SGSBASE)/alint/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) LPASS=$(LPASS)
	cd $(SGSBASE)/cpp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) PD_SYS=D_unix \
		PD_MACH=D_newmach NEW_MACH="-DPD_MY_MACH=\\\"$(MACH)\\\""
	cd $(SGSBASE)/cmd/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/optim/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/as/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/ld/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/dis/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/dump/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/lorder/i386; $(MAKE) $(ENVPARMS)
	cd $(SGSBASE)/lprof/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/m4/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/mcs/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/nm/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/size/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/cof2elf/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/strip/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/tsort/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/unix_conv/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/../make; /bin/make -f make.mk
	echo "Successfully built tools."
#
tools:
	cd $(SGSBASE)/yacc/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS)
	cd $(SGSBASE)/lex/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS)
#
libs:
	cd $(SGSBASE)/libelf/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(SGSBASE)/libld/i386; $(MAKE) $(ENVPARMS) $(CCPARMS)
#
install: toolinstall libinstall
	cd $(SGSBASE)/ar/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/acomp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/alint/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) LPASS=$(LPASS) install
	cd $(SGSBASE)/cpp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) install \
		PD_SYS=D_unix PD_MACH=D_newmach NEW_MACH="-DPD_MY_MACH=\\\"$(MACH)\\\""
	cd $(SGSBASE)/cmd/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/optim/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/as/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/ld/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/dis/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/dump/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/lorder/i386; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/lprof/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/m4/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/mcs/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/nm/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/size/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/strip/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/cof2elf/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/tsort/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/unix_conv/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cp $(SGSBASE)/../make/make $(CCSBIN)/$(SGS)make
	cp $(SGSBASE)/xenv/i386/env.tmp $(CCSBIN)/$(SGS)env
	cp $(SGSBASE)/xenv/i386/make.tmp $(CCSBIN)/make
	cp $(SGSBASE)/xenv/i386/vax $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/u3b $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/u370 $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/u3b15 $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/u3b5 $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/u3b2 $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/uts $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/i286 $(CCSBIN)
	cp $(SGSBASE)/xenv/i386/i386 $(CCSBIN)
	echo "Successfully installed tools."
#
toolinstall:
	cd $(SGSBASE)/yacc/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/lex/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	
#
libinstall:
	cd $(SGSBASE)/libelf/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(SGSBASE)/libld/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
#
libcopy:
	# save host version of libelf
	if [ -n "$(SGS)" ] ; \
	then \
		mv $(CCSLIB)/libelf.a $(CCSLIB)/libelf$(SGS).a;\
	fi
#
lintit:	toollintit liblintit
	cd $(SGSBASE)/ar/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/acomp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) lintit 
	cd $(SGSBASE)/cpp/i386; $(MAKE) $(ENVPARMS) $(CPPARMS) lintit
	cd $(SGSBASE)/cmd/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/optim/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/as/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/ld/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/dis/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/dump/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/alint/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/lorder/i386; $(MAKE) $(ENVPARMS) lintit
	cd $(SGSBASE)/lprof/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/m4/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/mcs/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/nm/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/size/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/strip/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/cof2elf/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/tsort/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/unix_conv/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit
	cd $(SGSBASE)/../make; /bin/make -f make.mk lintit
#
toollintit:
	cd $(SGSBASE)/yacc/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit 
	cd $(SGSBASE)/lex/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) lintit 
#
liblintit:
	cd $(SGSBASE)/libelf/i386; $(MAKE) $(ENVPARMS) lintit
	cd $(SGSBASE)/libld/i386; $(MAKE) $(ENVPARMS) lintit
#
clean: toolclean libclean
	cd $(SGSBASE)/ar/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clean
	cd $(SGSBASE)/acomp/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/cpp/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/cmd/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/optim/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/as/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/ld/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/dis/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/dump/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/alint/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/lorder/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/lprof/i386; $(MAKE) $(ENVPARMS) clean 
	cd $(SGSBASE)/m4/i386; $(MAKE) $(ENVPARMS) clean 
	cd $(SGSBASE)/mcs/i386; $(MAKE) $(ENVPARMS) clean 
	cd $(SGSBASE)/nm/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/size/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/strip/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/cof2elf/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/tsort/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/unix_conv/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/../make; /bin/make -f make.mk clean
#
toolclean:
	cd $(SGSBASE)/yacc/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clean 
	cd $(SGSBASE)/lex/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clean 
#
libclean:
	cd $(SGSBASE)/libelf/i386; $(MAKE) $(ENVPARMS) clean
	cd $(SGSBASE)/libld/i386; $(MAKE) $(ENVPARMS) clean
#
clobber: toolclobber libclobber
	cd $(SGSBASE)/ar/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clobber
	cd $(SGSBASE)/acomp/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/cpp/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/cmd/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/optim/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/as/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/ld/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/dis/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/dump/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/alint/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/lorder/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/lprof/i386; $(MAKE) $(ENVPARMS) clobber 
	cd $(SGSBASE)/m4/i386; $(MAKE) $(ENVPARMS) clobber 
	cd $(SGSBASE)/mcs/i386; $(MAKE) $(ENVPARMS) clobber 
	cd $(SGSBASE)/nm/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/size/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/strip/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/cof2elf/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/tsort/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/unix_conv/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/../make; /bin/make -f make.mk clobber
#
toolclobber:
	cd $(SGSBASE)/yacc/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clobber 
	cd $(SGSBASE)/lex/i386; $(MAKE) $(ENVPARMS) $(CCPARMS) clobber 
#
libclobber:
	cd $(SGSBASE)/libelf/i386; $(MAKE) $(ENVPARMS) clobber
	cd $(SGSBASE)/libld/i386; $(MAKE) $(ENVPARMS) clobber
