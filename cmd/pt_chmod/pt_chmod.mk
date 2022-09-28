#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)pt_chmod:pt_chmod.mk	1.1.2.1"

ROOT=
USRLIB=$(ROOT)/usr/lib
INC=$(ROOT)/usr/include
CFLAGS= -I$(INC)
LDFLAGS = -s

OBJS= pt_chmod.o

#
# Header dependencies
#

INCLUDE= $(INC)/grp.h

all:
	$(CC) $(CFLAGS) $(LDFLAGS) pt_chmod.c -o pt_chmod

$(OBJS):	$(INCLUDES)

install:  all
	install -f $(USRLIB) pt_chmod
	$(CH)chgrp bin  $(USRLIB)/pt_chmod
	$(CH)chmod 04111 $(USRLIB)/pt_chmod
	$(CH)chown root $(USRLIB)/pt_chmod

clean:
	-rm -f *.o

clobber:	clean
	-rm -f pt_chmod

