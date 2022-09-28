#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)lp:cmd/lpsched/lpNet/svChild/Makefile	1.7.2.1"

ROOT	=
LPROOT	=	../../../..
LPNETROOT=	..
LPLIB	=	$(LPROOT)/lib
LPINC	=	$(LPROOT)/include
BIN	=	$(ROOT)/bin
USRBIN	=	$(ROOT)/usr/bin
USRLIB	=	$(ROOT)/usr/lib
INC	=	$(ROOT)/usr/include
INCSYS	=	$(INC)/sys
INCDIR	=	../include

MAKE	=	make
CC	=	cc
AS	=	as
LORDER	=	lorder
TSORT	=	tsort
AR	=	ar
LINT	=	lint
ENV	=

DEBUG	=	-DDEBUG
CFLAGS	=	-I$(INCDIR) -I$(LPINC) -I$(INC) $(DEBUG)
LFLAGS	=	-I$(INCDIR) -I$(LPINC) -I$(INC)
ASFLAGS	=
ARFLAGS	=	rv


LIB	=	svChild.a
TARGETS	=	$(LIB)

.PRECIOUS:	$(TARGETS)

SRCS	=	svChild.c  getjobfiles.c
OBJS	=	$(SRCS:.c=.o)

.c.o:
		$(CC) $(CFLAGS) -c $*.c

all:		$(LIB)
		@for t in $(TARGETS);			\
		do					\
			echo "$${t} is-up-to-date.";	\
		done

install:	all

clean:
		rm -f *.o

clobber:	clean
		rm -f $(LIB)

lint:
		$(LINT) $(LFLAGS) $(SRCS)  >lint.out 2>&1

product:
		@for t in $(TARGETS);	\
		do			\
			echo $${t};	\
		done


$(LIB):		$(OBJS)
		$(AR) $(ARFLAGS) $(LIB) `$(LORDER) *.o | $(TSORT)`

svChild.o:	$(INCDIR)/lpNet.h

getjobfiles.o:
