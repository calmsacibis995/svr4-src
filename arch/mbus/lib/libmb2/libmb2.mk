#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:lib/libmb2/libmb2.mk	1.3"

ROOT =
USRLIB	= $(ROOT)/usr/lib
LIB	= $(ROOT)/lib
INC	= $(ROOT)/usr/include
CFLAGS	= -I$(INC) -O

LIBSRC	= _mb2_utility.c \
	  mb2a_prim.c \
	  mb2s_prim.c\
	  icslib.c

LIBRARY	= libmb2.a

LIBOBJS	= $(LIBRARY)(_mb2_utility.o) \
	  $(LIBRARY)(mb2a_prim.o) \
	  $(LIBRARY)(mb2s_prim.o) \
	  $(LIBRARY)(icslib.o)

INCLUDES = \
	  $(INC)/sys/param.h \
	  $(INC)/sys/types.h \
	  $(INC)/sys/stat.h \
	  $(INC)/sys/fcntl.h \
	  $(INC)/sys/errno.h \
	  $(INC)/sys/stream.h \
	  $(INC)/sys/stropts.h \
	  $(ROOT)/usr/include/sys/mps.h \
	  $(ROOT)/usr/include/sys/mb2taiusr.h \
		$(INC)/sys/param.h \
		$(INC)/sys/fs/s5dir.h \
		$(INC)/sys/signal.h \
		$(INC)/sys/user.h \
		$(INC)/sys/errno.h \
		$(ROOT)/usr/include/sys/ics.h \
		$(INC)/stdio.h \
	  _def.h


all:	$(LIBRARY)

$(LIBRARY):	$(LIBOBJS)

install: all
	install -u bin -g bin -m 664 -f $(USRLIB) libmb2.a

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBRARY)

$(LIBOBJS): $(INCLUDES)
