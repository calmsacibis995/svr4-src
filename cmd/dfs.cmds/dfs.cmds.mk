#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dfs.cmds:dfs.cmds.mk	1.5.3.1"
#
# makefile for dfs.cmds
#
# These are the generic distributed file system administration commands
#

ROOT =
TESTDIR = .
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O
COMMANDS=general dfshares share shareall unshareall
GENERAL=unshare
FRC =

all:
	@for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;

install:
	for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make install -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;
	for i in $(GENERAL);\
		do \
			rm -f $(ROOT)/usr/sbin/$$i;\
			ln $(ROOT)/usr/sbin/general $(ROOT)/usr/sbin/$$i;\
		done
	rm -f $(ROOT)/usr/sbin/dfmounts
	ln $(ROOT)/usr/sbin/dfshares $(ROOT)/usr/sbin/dfmounts

clean:
	for i in $(COMMANDS);\
	do\
		cd $$i;\
		echo $$i;\
		make -f $$i.mk clean;\
		cd .. ;\
	done

clobber: clean
	for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make -f $$i.mk clobber;\
		cd .. ;\
	done

FRC:
