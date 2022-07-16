#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1989  Intel Corporation
#	All Rights Reserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/reset.d/reset.mk	1.3"

DASHO = -O

DIR = $(ROOT)/usr/lbin

INC = $(ROOT)/usr/include

LOCALLIBS = -lmb2 -lx

SHLIBS =

CFLAGS = $(DASHO) -I$(INC) $(MORECPP) 

CC = cc

SRCS = reset.c util.c

OBJS = $(SRCS:.c=.o)

MAINS = reset

.PRECIOUS: $(SRCS)

all: $(MAINS)

install: all dir
	for i in $(MAINS)                                ;\
	do                                                \
		install -f $(DIR) -m 0700 -g sys -u root $$i ;\
	done

dir:
	-@[ -d $(DIR) ]     || mkdir $(DIR)

$(MAINS):
	$(CC) -o $@ $? $(CCFLAGS) $(LDFLAGS) $(LOCALLIBS)  $(SHLIBS)

clean :
	rm -f $(OBJS)

clobber :
	rm -f $(OBJS) $(MAINS)

reset.o : reset.c reset.h

util.o : util.c reset.h

reset : $(OBJS)

lint:
	lint $(CFLAGS) $(SRCS)
