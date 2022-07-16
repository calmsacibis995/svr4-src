#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libm:libm.mk	1.17.2.13"
#
# makefile for libm
#
# By default, both profiled and non-profiled libraries are built.
# To build only the non-profiled version, set ARCHIVE=nonprofarch
# To build only the profiled version, set ARCHIVE=profarch
#
# If using a non-ANSI compiler, only double precision functions are
# built.
#
ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../../cmd/sgs
INS=$(SGSBASE)/sgs.install
INSDIR=
STRIP=strip
HFILES=
SOURCES=
OBJECTS=
PRODUCTS=
CCSLIB=$(ROOT)/usr/ccs/lib
ARCHIVE=archive
DEFLIST=
MAKE=make
INSPDIR=
CCS=ALL
ENVPARMS=OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" CFLAGS="$(CFLAGS)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)"  STRIP="$(STRIP)" MAKE="$(MAKE)" CCSLIB="$(CCSLIB)" 

#
# By default, both INLINE and FPE versions of the 3b2 libm
# are built.  This behaviour can be altered by setting
# the parameter CCS to INLINE for only the inline version, or to
# anything but ALL or INLINE for the FPE version
#

all:
	if u3b15 || u3b5 || u3b2 ;\
		then if [ $(CCS) = "ALL" ] ;\
			then cd m32mau; $(MAKE)  $(ARCHIVE) $(ENVPARMS);\
			cd ..;\
			cd m32; $(MAKE)  $(ARCHIVE) $(ENVPARMS);\
			cd ..;\
			cd m32_sfm; $(MAKE)  $(ARCHIVE)  $(ENVPARMS);\
		else if [ $(CCS) = "INLINE" ] ;\
			then cd m32mau; $(MAKE)  $(ARCHIVE) $(ENVPARMS); \
			cd ..;\
			else cd m32; $(MAKE)  $(ARCHIVE) $(ENVPARMS);\
			cd ..;\
		fi; \
		fi; \
	else if i386; \
		then cd i386; $(MAKE)  $(ARCHIVE)  $(ENVPARMS); \
		cd ..;\
		fi; \
		fi;

nonansi:
	if u3b15 || u3b5 || u3b2 ;\
		then cd m32; $(MAKE) $(ENVPARMS) nonansi_arch ;\
		cd ..;\
	else if i386; \
		then cd i386; $(MAKE)  $(ENVPARMS) nonansi_arch ANSIDEF=; \
		cd ..;\
		fi; \
	fi; 
move:
	#
	if u3b15 || u3b5 || u3b2 ;\
		then if [ $(CCS) = "ALL" ] ;\
			then cd m32mau; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)/fp" INSPDIR="$(CCSLIB)/fp/libp"; \
			cd .. ; \
			cd m32; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)" INSPDIR="$(CCSLIB)/libp"; \
			cd .. ;\
			cd m32_sfm; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)/fp" ; \
		else if [ $(CCS) = "INLINE" ] ;\
			then cd m32mau; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)/fp" INSPDIR="$(CCSLIB)/fp/libp"; \

		else cd m32; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)" INSPDIR="$(CCSLIB)/libp"; \
		fi; \
		fi; \
	else if i386; \
		then cd i386; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)" INSPDIR="$(CCSLIB)/libp"; \
		fi; \
		fi;

nonansi_move:
	#
	if u3b15 || u3b5 || u3b2 ;\
		then cd m32; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)" INSPDIR="$(CCSLIB)/libp"; \
	else if i386; \
		then cd i386; $(MAKE) move $(ENVPARMS) INSDIR="$(CCSLIB)" INSPDIR="$(CCSLIB)/libp"; \
		fi; \
	fi ;

install: 
	- $(CC) -E tstansi.c; if [ $$? = 0 ] ; \
	then $(MAKE) -f libm.mk all move $(ENVPARMS) ;\
	else $(MAKE) -f libm.mk nonansi nonansi_move $(ENVPARMS) ;\
	fi ; 

clean:
	#
	# remove intermediate files except object modules and temp library
	if u3b15 || u3b5 || u3b2 ;\
		then if [ $(CCS) = "ALL" ] ;\
			then cd m32mau; $(MAKE) clean ; \
			cd .. ; \
			cd m32; $(MAKE) clean ; \
			cd .. ; \
			cd m32_sfm; $(MAKE) clean ; \
		else if [ $(CCS) = "INLINE" ] ;\
			then cd m32mau; $(MAKE) clean ; \
		else cd m32; $(MAKE) clean ; \
		fi; \
		fi; \
	else if i386; \
		then cd i386; $(MAKE) clean; \
		fi; \
		fi;
		#
clobber:
	#
	# remove intermediate files
	if u3b15 || u3b5 || u3b2 ;\
		then if [ $(CCS) = "ALL" ] ;\
			then cd m32mau; $(MAKE) clobber ; \
			cd .. ; \
			cd m32; $(MAKE) clobber ; \
			cd .. ; \
			cd m32_sfm; $(MAKE) clobber ; \
		else if [ $(CCS) = "INLINE" ] ;\
			then cd m32mau; $(MAKE) clobber ; \
		else cd m32; $(MAKE) clobber ; \
		fi; \
		fi; \
	else if i386; \
		then cd i386; $(MAKE) clobber; \
		fi; \
		fi;
		#

lintit:
	#
	if u3b2 || u3b5 || u3b15; \
		then cd port ; $(MAKE) lintit LINT=$(LINT) LINTFLAGS=$(LINTFLAGS) ROOT=$(ROOT) DEFLIST=-DM32 ;\
	else if i386; \
		then cd port ; $(MAKE) lintit LINT=$(LINT) LINTFLAGS=$(LINTFLAGS) ROOT=$(ROOT) ; \
	fi; \
	fi;
