#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:cmd/bsd/lprm/Makefile	1.10.3.1"


TOP	=	../../..

include ../../../common.mk


TAGS	=	ctags

USRUCB	=	$(ROOT)/usr/ucb


DEBUG	=	-DDEBUG

CFLAGS	=	$(DEBUG) -O -I$(LPINC) -I$(INC)

LDFLAGS	=	$(SHLIBS)


SRCS	= \
		lprm.c

OBJS	=	$(SRCS:.c=.o)


CMD	=	lprm


all:		$(CMD)

install:	all
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRUCB) $(CMD)

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMD)

strip:
	$(STRIP) $(CMD)

lprm:		$(OBJS) \
			$(LIBBSD) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lprm $(OBJS) $(LDFLAGS) \
			$(LIBBSD) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP) \
				-lgen

stamp:

lint:
	$(LINT) $(CFLAGS) $(SRCS) \
			$(LINTREQ) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)

ctags:
	$(TAGS) $(SRCS)
