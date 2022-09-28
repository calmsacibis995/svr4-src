#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:cmd/lpdata/Makefile	1.6.3.1"
#
# Makefile for the lpdata command
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-g -DDEBUG

CFLAGS	=	-I. -I$(LPINC) -I$(INC) $(DEBUG)

LDFLAGS	=	$(SHLIBS)


SRCS	= \
		lpdata.c

OBJS	=	$(SRCS:.c=.o)

LIBS	= \
		$(LIBMSG) \
		$(LIBFRM) \
		$(LIBREQ) \
		$(LIBPRT) \
		$(LIBCLS) \
		$(LIBACC) \
		$(LIBFLT) \
		$(LIBUSR) \
		$(LIBOAM) \
		$(LIBLP)

SYSLIBS	=	-lgen

LINTS	= \
		$(LINTMSG) \
		$(LINTFRM) \
		$(LINTREQ) \
		$(LINTPRT) \
		$(LINTCLS) \
		$(LINTACC) \
		$(LINTFLT) \
		$(LINTUSR) \
		$(LINTOAM) \
		$(LINTLP)


CMD	=	lpdata


all:		$(CMD)

install:
	$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRLIBLP) $(CMD)

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMD)

strip:
	$(STRIP) $(CMD)

$(CMD):		$(OBJS) $(LIBS)
	$(CC) -o $(CMD) $(OBJS) $(LDFLAGS) $(LIBS) $(SYSLIBS)

lint:
	lint -x $(CFLAGS) $(SRCS) $(LINTS)
