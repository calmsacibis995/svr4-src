#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nametoaddr:nametoaddr.mk	1.6.2.2"
#
# makefile for name to address mapping dynamic linking libraries.
#

ROOT =
TESTDIR = .
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O
FRC =

all:
	@for i in * ; do  \
		if [ -d $$i ] ; then \
			cd $$i;\
			echo "##### make -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\"" ;\
			make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
			cd ..; \
		fi; \
	done;

install: 
	@for i in * ; do  \
		if [ -d $$i ] ; then \
			cd $$i;\
			echo $$i;\
			echo "##### make install -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\"" ;\
			make install -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
			cd ..;\
		fi; \
	done;

clean:
	@for i in * ; do  \
		if [ -d $$i ] ; then \
			cd $$i;\
			echo $$i;\
			make -f $$i.mk clean;\
			cd .. ;\
		fi; \
	done

clobber:
	@for i in * ; do  \
		if [ -d $$i ] ; then \
			cd $$i;\
			echo $$i;\
			make -f $$i.mk clobber;\
			cd .. ;\
		fi; \
	done

FRC:
