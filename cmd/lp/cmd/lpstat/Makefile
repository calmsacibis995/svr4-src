#ident	"@(#)Makefile	1.2	91/09/22	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:cmd/lpstat/Makefile	1.17.3.1"
#
# Makefile for the lpstat command
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-DDEBUG

CFLAGS	=	$(DEBUG) -O -I. -I$(LPINC) -I$(INC)

LDFLAGS	=	$(SHLIBS)


HDRS	= \
		lpstat.h

SRCS	= \
		accept.c \
		add_mounted.c \
		charset.c \
		class.c \
		device.c \
		done.c \
		form.c \
		lpstat.c \
		output.c \
		parse.c \
		printer.c \
		request.c \
		send_message.c

OBJS	=	$(SRCS:.c=.o)

LIBS	= \
		$(LIBACC) \
		$(LIBCLS) \
		$(LIBMSG) \
		$(LIBPRT) \
		$(LIBFRM) \
		$(LIBOAM) \
		$(LIBLP)

SYSLIBS =	-lcurses

LINTS	= \
		$(LINTACC) \
		$(LINTCLS) \
		$(LINTMSG) \
		$(LINTPRT) \
		$(LINTFRM) \
		$(LINTOAM) \
		$(LINTLP)


CMD	=	lpstat


all:		$(CMD)

install:	all
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) $(CMD)

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMD)

strip:
	$(STRIP) $(CMD)

$(CMD):		$(OBJS) $(LIBS)
	$(CC) -o $(CMD) $(OBJS) $(LDFLAGS) $(LIBS) $(SYSLIBS)

$(OBJS):	$(HDRS)

lint:
	$(LINT) -x $(CFLAGS) $(SRCS) $(LINTS)
