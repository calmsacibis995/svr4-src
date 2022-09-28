#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/bsd/Makefile	1.7.2.1"
#
# Makefile for bsd routines
#


TOP	=	../..

include ../../common.mk


DEBUG	=	-DDEBUG
XCFLAGS	=	-O
CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) $(XCFLAGS)


SRCS	= \
		displayq.c \
		escape.c \
		fatalmsg.c \
		findfld.c \
		gethostnm.c \
		global.c \
		log.c \
		parse.c \
		psfile.c \
		rmjob.c \
		rsendjob.c \
		sndrcv.c

OBJS	=	$(SRCS:.c=.o)

LIBNAME	=	lpbsd

include ../libcommon.mk
