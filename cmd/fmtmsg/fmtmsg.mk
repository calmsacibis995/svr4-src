#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fmtmsg:fmtmsg.mk	1.12.2.1"

INC=$(ROOT)/usr/include
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LIBS=
INSTALL=install
BIN=$(ROOT)/usr/bin
HDRS=$(INC)/fmtmsg.h $(INC)/stdio.h $(INC)/string.h $(INC)/errno.h
LCLHDRS=
FILE=fmtmsg
INSTALLS=fmtmsg
SRC=main.c
OBJ=$(SRC:.c=.o)
CFLAGS=-I . -I $(INC) $(CCFLAGS)
LDFLAGS=-s

all		: $(FILE) 

install		: all
		$(INSTALL) -f $(BIN) $(INSTALLS)

clobber		: clean
		rm -f $(FILE)

clean		:
		rm -f $(OBJ)

strip		:
		$(CC) -O -s $(FILE).o -o $(FILE) $(LDLIBPATH) $(CFLAGS)

lintit		: $(SRC)
		lint $(CFLAGS) $(SRC)

$(FILE)		: $(OBJ) $(LIBS)
		$(CC) -O $(OBJ) -o $(FILE) $(LDLIBPATH) $(CFLAGS) $(LDFLAGS)

$(OBJ)		: $(HDRS)
