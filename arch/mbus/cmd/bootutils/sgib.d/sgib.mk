#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988, 1989  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/bootutils/sgib.d/sgib.mk	1.3"

BUS 	= MB2
DASHO 	= -O
INC		=$(ROOT)/usr/include
LDFLAGS =   
LDADD	= -lld -lelf
CFLAGS	=  $(DASHO) -I$(INC) -D$(BUS)
SRC		= 	sgib.c mkboot.c mkbtblk.c util.c
OBJS	=	$(SRC:.c=.o)
MAIN	=	sgib

all: $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) -o sgib $(LDADD) $(LDFLAGS) $(SHLIBS)

install: all
	install -f $(ROOT)/sbin -m 700 -u root -g bin $(MAIN)

lint:
	lint $(SRC)
clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(MAIN)

sgib.o: sgib.h 
mkbtblk.o: sgib.h 
mkboot.o: sgib.h 
util.o: sgib.h 
