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

#ident	"@(#)mbus:cmd/mbusutils/misc.d/misc.mk	1.3.1.1"

DASHO = -O
BUS = MB2
ARCH = MBUS
DIR  = $(ROOT)/usr/lbin
SBIN  = $(ROOT)/sbin
INC = $(ROOT)/usr/include
LIBS=-lmb2 -lbps
LDFLAGS = -s $(DASHO) 
CFLAGS = -I$(INC) $(SHLIBS) -D$(BUS) -D$(ARCH)
STRIP = strip
SIZE = size
LIST = lp
MAKEFILE = misc.mk

SCRIPTS	=	

SOURCES = 	cpout.c cpunum.c mpscnv.c iasyports.c

OBJECTS =	$(SOURCES:.c=.o)
MAINS =		$(SOURCES:.c=)

.PRECIOUS: $(SOURCES) 

all: $(MAINS)
	@echo "\n****** misc.d  completed ******"

install: all dir $(SCRIPTS) $(SOURCES) 
	for i in $(MAINS)	;\
	do							 \
		install -f $(SBIN) -m 0700 -u root -g sys $$i	;\
	done
	for i in mpscnv qtty	;\
	do							 \
		install -f $(DIR) -m 0700 -u root -g sys $$i	;\
	done

dir:
	-@[ -d $(DIR) ] || mkdir $(DIR)
	-@[ -d $(SBIN) ] || mkdir $(SBIN)

mpscnv: mpscnv.c
	$(CC)  -O $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBS) -ll

iasyports: iasyports.c
	$(CC)  -O $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBS) -lelf

cpout cpunum:
	$(CC)  -O $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBS)

clean:
	rm -f $(SCRIPTS) $(OBJS)  mpscnv.o mpscnv.c
clobber: clean
	rm -f $(MAINS)

FRC:
