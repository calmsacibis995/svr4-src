#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/access/Makefile	1.12.2.1"
#
# Makefile for accessmgmt(lp) routines
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC)


SRCS	= \
		allowed.c \
		bang.c \
		change.c \
		dumpaccess.c \
		files.c \
		loadaccess.c

OBJS	=	$(SRCS:.c=.o)

LINTS	= \
		$(LINTLP)

LIBNAME	=	lpacc

include ../libcommon.mk
