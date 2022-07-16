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

#ident	"@(#)mbus:cmd/bootutils/hdformat.d/hdformat.mk	1.3"

BUS 	= MB2
DASHO 	= -O
INC		=$(ROOT)/usr/include
SHLIBS =
CFLAGS	=  $(DASHO) -I$(INC) -D$(BUS) $(SHLIBS)
SRC		= 	hdformat.c  mdl.c
OBJS	=	$(SRC:.c=.o)
MAINS	=	$(SRC:.c=)

all: $(MAINS)

hdformat: hdformat.c
	$(CC) $(CFLAGS) -o hdformat hdformat.c $(LDFLAGS) 

mdl: mdl.c
	$(CC) $(CFLAGS) -o mdl mdl.c $(LDFLAGS)

install: all
	install -f $(ROOT)/sbin -m 700 -u root -g bin hdformat
	install -f $(ROOT)/sbin -m 700 -u root -g bin mdl

lint:
	lint $(SRC)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(MAINS)
