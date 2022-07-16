#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libnls:libnls.mk	1.2.2.1"
#
# libnls.mk: makefile for network listener library
#

INC	= $(ROOT)/usr/include
OPT	= -O -s
# if debug is needed then add -DDEBUGMODE to following line
CFLAGS	= -I$(INC) ${OPT} 
LDFLAGS	=
LIBNLS = libnls.a
LINT = lint
LINTFLAGS = -b -x

# change the next line to compile with -g
# OPT	= -g

LIB = $(ROOT)/usr/lib

LIBID	= bin
INCID	= bin

SRC = nlsenv.c nlsdata.c nlsrequest.c

OBJ = $(LIBNLS)(nlsenv.o) $(LIBNLS)(nlsdata.o) $(LIBNLS)(nlsrequest.o)

all:	libnls


lintit:
		$(LINT) $(LINTFLAGS) $(SRC)

libnls:	$(LIBNLS)

$(LIBNLS):	$(OBJ)

.PRECIOUS:	$(LIBNLS)

# listener library routines and /usr/include headers:

$(LIBNLS)(nlsenv.o):	$(INC)/ctype.h $(INC)/listen.h $(INC)/sys/tiuser.h
$(LIBNLS)(nlsdata.o):	$(INC)/sys/tiuser.h
$(LIBNLS)(nlsrequest.o):	$(INC)/stdio.h $(INC)/ctype.h $(INC)/fcntl.h \
				$(INC)/errno.h $(INC)/string.h $(INC)/sys/tiuser.h \
				$(INC)/listen.h

install:	all
		install -f $(LIB) -u $(LIBID) -g $(LIBID) -m 644 $(LIBNLS)

clean:
	-rm -f *.o

clobber: clean
	-rm -f libnls.a

FRC:
