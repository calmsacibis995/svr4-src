#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)librpcsvc:librpcsvc.mk	1.7.5.1"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
# 
#
USRLIB = $(ROOT)/usr/lib
INC = $(ROOT)/usr/include 
INS = install
HDRS = rusers.h rwall.h spray.h
DESTINCLUDE = $(ROOT)/usr/include/rpcsvc
LIBNAME = librpcsvc.a
CFLAGS= -O -I$(INC) 
LORDER = lorder
OBJS= rusersxdr.o rwallxdr.o sprayxdr.o \
      klm_prot.o nlm_prot.o sm_inter.o
SRCS = $(OBJS:%.o=%.c)

all: $(USRLIB)/$(LIBNAME)

$(USRLIB)/$(LIBNAME): $(OBJS)
	rm -f $(LIBNAME);
	$(AR) cr $(LIBNAME) $(OBJS) 

$(LIBNAME): $(OBJS)
	$(AR) cr $(LIBNAME) `$(LORDER) $(OBJS) | tsort `

install: $(USRLIB)/$(LIBNAME)
	$(INS) -f $(USRLIB) $(LIBNAME)

installhdrs: $(HDRS)
	$(INS) -f  $(DESTINCLUDE) $(HDRS)

lint:
	$(LINT) $(CFLAGS) $(SRCS)  

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(LIBNAME)
