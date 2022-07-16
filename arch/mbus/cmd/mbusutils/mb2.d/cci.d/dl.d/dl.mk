#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/dl.mk	1.3"

DASHO = -O

DEFDIR = $(ROOT)/etc/default

ULIB = $(ROOT)/usr/lib

BIN = $(ULIB)/cci

LOCALLIBS = -lcmd -lx

SHLIBS = -dy

INC = $(ROOT)/usr/include

CFLAGS = $(DASHO) -I$(INC) -DMBIIU $(MORECPP)

CC = cc

SRCS = down.c com.c util.c record.c interface.c

OBJS = $(SRCS:.c=.o)

MAINS = download

.PRECIOUS: $(SRCS)

install: all dirs
	for i in $(MAINS)                                ;\
	do                                                \
		install -f $(BIN) -m 0700 -g sys -u root $$i ;\
	done

	cp download.dfl $(ROOT)/etc/default/download

all: $(MAINS)

dirs:
	-@[ -d $(ULIB) ]    || mkdir $(ULIB)
	-@[ -d $(BIN) ]     || mkdir $(BIN)
	-@[ -d $(DEFDIR) ]  || mkdir $(DEFDIR)

$(MAINS):
	$(CC) -o $@ $? $(LDFLAGS) -lmb2  $(LOCALLIBS) $(SHLIBS)

clean :
	rm -f $(OBJS)

clobber :
	rm -f $(OBJS) $(MAINS)

down.o : down.c trans.h ext.h coff.h

com.o : com.c trans.h ext.h 410.h lit.h

util.o : util.c trans.h ext.h 410.h

record.o : record.c ext.h

interface.o : interface.c ext.h xprt.h 

download : $(OBJS)
