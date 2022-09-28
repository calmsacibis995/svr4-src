#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)su:su.mk	1.9.9.1"
#	Copyright (c) 1987, 1988 Microsoft Corporation	
#	  All Rights Reserved	
#	This Module contains Proprietary Information of Microsoft  
#	Corporation and should be treated as Confidential.	   
#	su make file
ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/sbin
LDFLAGS = -O -s -lcrypt_i -lcmd
INS = install

all:	su

su:
	$(CC) -I$(INC) -o su su.c $(LDFLAGS) $(ROOTLIBS)


install: su
	$(INS) -f $(INSDIR) -m 4555 -u root -g sys su
	$(INS) -f $(ROOT)/usr/bin -m 4555 -u root -g sys su
	-mkdir ./tmp
	-ln su.dfl ./tmp/su
	$(INS) -f $(ROOT)/etc/default -m 0444 -u root -g sys ./tmp/su
	-rm -rf ./tmp

clean:


clobber: clean
	rm -f su
