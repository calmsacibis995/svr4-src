#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:cmd-inet.mk	1.10.3.1"

#
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# 		PROPRIETARY NOTICE (Combined)
# 
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
# 
# 
# 
# 		Copyright Notice 
# 
# Notice of copyright on this source code product does not indicate 
# publication.
# 
# 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
# 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
# 	          All rights reserved.
#  
#

DASHO=		-O
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
INSTALL=	install

DIRS=		etc usr.bin usr.sbin


PASSECHO=	\"AR=$(AR)\" \"AS=$(AS)\" \"CC=$(CC)\" \"DASHO=$(DASHO)\" \
		\"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"INSTALL=$(INSTALL)\" \"LD=$(LD)\" \"LDFLAGS=$(LDFLAGS)\" \
		\"LEX=$(LEX)\" \"MAKE=$(MAKE)\" \
		\"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\" \
		\"YACC=$(YACC)\" \"SHLIBS=$(SHLIBS)\" \"SYMLINK=$(SYMLINK)\"

PASSTHRU=	"AR=$(AR)" "AS=$(AS)" "CC=$(CC)" "DASHO=$(DASHO)" \
		"FRC=$(FRC)" "INC=$(INC)" \
		"INSTALL=$(INSTALL)" "LD=$(LD)" "LDFLAGS=$(LDFLAGS)" \
		"LEX=$(LEX)" "MAKE=$(MAKE)" \
		"MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)" \
		"YACC=$(YACC)" "SHLIBS=$(SHLIBS)" "SYMLINK=$(SYMLINK)"

all:
	@for i in $(DIRS);\
	do\
		cd $$i;\
		echo "\n===== $(MAKE) -f $$i.mk all $(PASSECHO)";\
		$(MAKE) -f $$i.mk all $(PASSTHRU);\
		cd ..;\
	done;\
	wait

install:
	@for i in $(DIRS);\
	do\
		cd $$i;\
		echo "\n===== $(MAKE) -f $$i.mk install $(PASSECHO)";\
		$(MAKE) -f $$i.mk install $(PASSTHRU);\
		cd ..;\
	done;\
	wait

clean:
	@for i in $(DIRS);\
	do\
		cd $$i;\
		echo "\n===== $(MAKE) -f $$i.mk clean";\
		$(MAKE) -f $$i.mk clean;\
		cd ..;\
	done;\
	wait

clobber:
	@for i in $(DIRS);\
	do\
		cd $$i;\
		echo "\n===== $(MAKE) -f $$i.mk clobber";\
		$(MAKE) -f $$i.mk clobber;\
		cd ..;\
	done;\
	wait
