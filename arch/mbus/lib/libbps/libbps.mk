#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1989  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.
#

#ident	"@(#)mbus:lib/libbps/libbps.mk	1.1"

ROOT = ${ROOT:-}
USRLIB	= $(ROOT)/usr/lib
LIB	= $(ROOT)/lib
INC	= $(ROOT)/usr/include
CFLAGS	= -I$(INC) -O

LIBSRC	= libbps.c

LIBRARY	= libbps.a

LIBOBJS	= $(LIBRARY)(libbps.o) 

INCLUDES = \
	  $(INC)/sys/param.h \
	  $(INC)/sys/types.h \
	  $(INC)/sys/fcntl.h \
	  $(INC)/sys/bps.h \
	  $(INC)/string.h 

all:	$(LIBRARY)

$(LIBRARY):	$(LIBOBJS)

install: all
	echo $(ROOT)
	install -u bin -g bin -m 664 -f $(USRLIB) libbps.a

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBRARY)

$(LIBOBJS): $(INCLUDES)
