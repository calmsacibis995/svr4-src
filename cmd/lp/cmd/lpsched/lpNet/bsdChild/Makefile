#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)lp:cmd/lpsched/lpNet/bsdChild/Makefile	1.5.2.1"

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
LPNETINC=	$(LPNETROOT)/include

MAKE	=	make
CC	=	cc
AS	=	as
LORDER	=	lorder
TSORT	=	tsort
AR	=	ar
LINT	=	lint
ENV	=	

ASFLAGS	=
ARFLAGS	=	rv
DEBUG	=	-DDEBUG
CFLAGS	=	-I$(LPNETINC) -I$(LPINC) -I$(INC) $(DEBUG)
LFLAGS	=	-I$(LPNETINC) -I$(LPINC) -I$(INC)

LIB	=	bsdChild.a
TARGETS	=	$(LIB)

.PRECIOUS:	$(TARGETS)

SRCS	=	bsdChild.c	\
		cancel.c	\
		getstatus.c	\
		netutil.c	\
		printjob.c	\
		printreq.c	\
		recvjob.c

OBJS	=	$(SRCS:.c=.o)

$(LIB):		$(OBJS)
		$(AR) $(ARFLAGS) $(LIB) `$(LORDER) *.o | $(TSORT) 2>/dev/null`

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

