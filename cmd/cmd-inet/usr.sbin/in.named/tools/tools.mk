#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/in.named/tools/tools.mk	1.14.3.1"

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
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP -DDEBUG
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
INSTALL=	install
USRSBIN=	$(ROOT)/usr/sbin

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)
LIBS=		-lresolv -lsocket -lnsl $(SHLIBS)


PASSECHO=	\"AR=$(AR)\" \"AS=$(AS)\" \"CC=$(CC)\" \"DASHO=$(DASHO)\" \
		\"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"INSTALL=$(INSTALL)\" \"LD=$(LD)\" \"LEX=$(LEX)\" \
		\"LIB_OPT=$(LIB_OPT)\" \"MAKE=$(MAKE)\" \
		\"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\"  \
		\"YACC=$(YACC)\" \"SHLIBS=$(SHLIBS)\" \"SYMLINK=$(SYMLINK)\"

PASSTHRU=	"AR=$(AR)" "AS=$(AS)" "CC=$(CC)" "DASHO=$(DASHO)" \
		"FRC=$(FRC)" "INC=$(INC)" \
		"INSTALL=$(INSTALL)" "LD=$(LD)" "LEX=$(LEX)" \
		"LIB_OPT=$(LIB_OPT)" "MAKE=$(MAKE)" \
		"MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)"  \
		"YACC=$(YACC)" "SHLIBS=$(SHLIBS)" "SYMLINK=$(SYMLINK)"

all:		nstest lookdir

install:	nstest
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin nstest
		@cd nslookup;\
		echo "\n===== $(MAKE) -f nslookup.mk install $(PASSECHO)";\
		$(MAKE) -f nslookup.mk install $(PASSTHRU)

nstest:		nstest.o
		$(CC) -o nstest $(CFLAGS) $(LDFLAGS) nstest.o $(LIBS)

lookdir:
		@cd nslookup;\
		echo "\n===== $(MAKE) -f nslookup.mk all $(PASSECHO)";\
		$(MAKE) -f nslookup.mk all $(PASSTHRU)

clean:
		rm -f *.o
		@cd nslookup;\
		echo "\n===== $(MAKE) -f nslookup.mk clean $(PASSECHO)";\
		$(MAKE) -f nslookup.mk clean $(PASSTHRU)

clobber:
		rm -f *.o nstest
		@cd nslookup;\
		echo "\n===== $(MAKE) -f nslookup.mk clobber $(PASSECHO)";\
		$(MAKE) -f nslookup.mk clobber $(PASSTHRU)

FRC:

#
# Header dependencies
#

nstest.o:	nstest.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/arpa/nameser.h \
		$(INC)/resolv.h \
		$(FRC)
