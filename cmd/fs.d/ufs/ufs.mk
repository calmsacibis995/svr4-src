#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ufs.cmds:ufs/ufs.mk	1.3.3.1"
#  /usr/src/cmd/lib/fs/ufs is the directory of all ufs specific commands
#  whose executable reside in $(INSDIR1) and $(INSDIR2).

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/fs/ufs
INSDIR2= $(ROOT)/etc/fs/ufs
INS = install
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s



#
#  This is to build all the ufs commands
#
all:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
		cd  $$i;\
		$(MAKE) -f $$i.mk "INC= $(INC)" "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"  "INC=$(INC)" "SHLIBS=$(SHLIBS)" "ROOTLIBS=$(ROOTLIBS)" "ROOT=$(ROOT)" all ; \
		cd .. ; \
	    fi;\
	done

install:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
		cd $$i;\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"  "INC=$(INC)" "SHLIBS=$(SHLIBS)" "ROOTLIBS=$(ROOTLIBS)" "ROOT=$(ROOT)" "INS=$(INS)" install ; \
		cd .. ; \
		fi;\
	done

clean:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
			cd $$i;\
			$(MAKE) -f $$i.mk clean; \
			cd .. ; \
		fi;\
	done

clobber:
	@for i in *;\
	do\
	    if [ -d $$i -a -f $$i/$$i.mk ]; \
		then \
			cd $$i;\
			$(MAKE) -f $$i.mk clobber; \
			cd .. ; \
		fi;\
	done

