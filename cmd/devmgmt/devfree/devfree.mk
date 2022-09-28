#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devmgmt:devfree/devfree.mk	1.7.3.1"



INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LIBS=$(USRLIB)/libadm.a
INSTALL=install
BIN=$(ROOT)/usr/bin
HDRS=$(INC)/stdio.h $(INC)/string.h $(INC)/errno.h $(INC)/fmtmsg.h $(INC)/devmgmt.h
LCLHDRS=
FILE=devfree
INSTALLS=devfree
PROTO=../Prototype
SRC=main.c
OBJ=$(SRC:.c=.o)
CFLAGS=-I . -I $(INC) -ladm $(CCFLAGS)
LDFLAGS=-s

all		: $(FILE) 

install		: all
		@eval `sed -e '/^![^=]*=/!d' -e 's/^!//' $(PROTO)` ;\
		for object in $(INSTALLS) ;\
		do \
		    if entry=`grep "[ 	/]$$object[= 	]" $(PROTO)` ;\
		    then \
			set -- $$entry ;\
			path=`eval echo $$3` ;\
			if expr $$path : '[^/]' >/dev/null ;\
			then \
			    path=$(BASEDIR)/$$path ;\
			fi ;\
			dir=$(ROOT)`dirname $$path` ;\
			if [ ! -d $$dir ] ;\
			then \
			    mkdir -p $$dir ;\
			fi ;\
			$(INSTALL) -f $$dir -m $$4 -u $$5 -g $$6 $$object ;\
		    else \
			echo "Unable to install $$object" ;\
		    fi ;\
		done

clobber		: clean
		rm -f $(FILE)

clean		:
		rm -f $(OBJ)

strip		:
		$(CC) -O -s $(FILE).o -o $(FILE) $(LDLIBPATH) $(CFLAGS)

lintit		:
		for i in $(SRC); \
		do \
		    lint $(CFLAGS) $$i; \
		done

$(FILE)		: $(OBJ) $(LIBS)
		$(CC) -O $(OBJ) -o $(FILE) $(LDLIBPATH) $(CFLAGS) $(LDFLAGS)

$(OBJ)		: $(HDRS)
