#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/nfs.mk	1.9.6.1"
#
# makefile for nfs.cmds
#
# These are the nfs specific subcommands for the generic distributed file
# system administration commands, along with many other nfs-specific
# administrative commands
#

ROOT =
TESTDIR = .
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O
DASHO = -O
COMMANDS=automount biod bootpd dfmounts dfshares exportfs mount mountd nfsd share showmount umount unshare statd lockd nfsstat pcnfsd
FRC =

all:
	@for i in $(COMMANDS);\
		do cd $$i;\
		echo "====> $(MAKE) -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\"";\
		make -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;

install: 
	@for i in $(COMMANDS);\
		do cd $$i;\
		echo "====> $(MAKE) -f $$i.mk install \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"SYMLINK=$(SYMLINK)\" \"INS=$(INS)\" \"INC=$(INC)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\"";\
		make -f $$i.mk install "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "SYMLINK=$(SYMLINK)" "INS=$(INS)" "INC=$(INC)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)";\
		cd ..;\
	done;

clean:
	@for i in $(COMMANDS);\
	do\
		cd $$i;\
		echo $$i;\
		make -f $$i.mk clean;\
		cd .. ;\
	done

clobber:
	@for i in $(COMMANDS);\
		do cd $$i;\
		echo $$i;\
		make -f $$i.mk clobber;\
		cd .. ;\
	done

FRC:
