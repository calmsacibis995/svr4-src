#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/msgs/Makefile	1.18.3.1"
#
# Makefile for messagemgmt(lp) routines
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC)


SRCS	= \
		_getmessage.c \
		_putmessage.c \
		getmessage.c \
		hslconv.c \
		putmessage.c \
		msgfmts.c \
		mclose.c \
		mconnect.c \
		mdisconnect.c \
		mgetputm.c \
		mlisten.c \
		mcreate.c \
		mdestroy.c \
		mneeds.c \
		mopen.c \
		mread.c \
		mrecv.c \
		msend.c \
		mwrite.c \
		streamio.c \
		fifo_buffs.c \
		read_fifo.c \
		write_fifo.c

OBJS	=	$(SRCS:.c=.o)

LINTS	= \
		$(LINTLP)

LIBNAME	=	lpmsg

include ../libcommon.mk

$(OBJS):	$(LPINC)/msgs.h
