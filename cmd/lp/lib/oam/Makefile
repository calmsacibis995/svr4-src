#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/oam/Makefile	1.15.3.1"
#
# Makefile for LP message management routines
#


TOP	=	../..

include ../../common.mk


##########
#
# Define the following if the message formatter should break long
# messages at word boundaries. The alternative is to break at a fixed
# length.
##########
SMARTDEF	=	-DSMART_WRAP

DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) $(SMARTDEF)


TEXTSRC	= \
		e_adm__msgs.c \
		e_can__msgs.c \
		e_cmb__msgs.c \
		e_fl__msgs.c \
		e_for__msgs.c \
		e_lp__msgs.c \
		e_lpp__msgs.c \
		e_lpu__msgs.c \
		e_mov__msgs.c \
		e_sht__msgs.c \
		e_stat__msgs.c \
		e_sys__msgs.c

SRCS	= \
		agettxt.c \
		buffers.c \
		fmtmsg.c

OBJS	=	$(SRCS:.c=.o) $(TEXTSRC:.c=.o)

LIBNAME	=	lpoam

include ../libcommon.mk


clean::	
	$(RM) $(TEXTSRC) $(LPINC)/oam_def.h xx??

$(SRCS:.c=.o):	$(LPINC)/oam_def.h

$(LPINC)/oam_def.h:	msg.source \
			gen-defs \
			gen-text
	sh gen-defs >$(LPINC)/oam_def.h
	sh gen-text
