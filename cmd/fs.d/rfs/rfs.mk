#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfs.cmds:rfs.cmds.mk	1.6.3.1"


#
# makefile for rfs.cmds
#
# These are the rfs specific subcommands for the generic distributed file
# system administration commands
#

ROOT =
TESTDIR = .
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
CFLAGS = -O
COMMANDS=dfmounts dfshares mount share unshare
FRC =

all:
	@for i in $(COMMANDS);\
		do cd $$i;\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "INCSYS=$(INCSYS)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;

install: 
	for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make install -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "INCSYS=$(INCSYS)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;

clean:
	for i in $(COMMANDS);\
	do\
		cd $$i;\
		echo $$i;\
		make -f $$i.mk clean;\
		cd .. ;\
	done

clobber:
	for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make -f $$i.mk clobber;\
		cd .. ;\
	done

FRC:
