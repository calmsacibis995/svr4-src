#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/bin.mk	1.3"

DASHO = -O

ULIB = $(ROOT)/usr/lib

BIN = $(ULIB)/cci

INC = $(ROOT)/usr/include

CFLAGS = $(DASHO) -I$(INC) $(MORECPP)

CC = cc

SRCS = ccisrvinfo.c cciload.c ccildlist.c ccildinfo.c ccilinfo.c ccisubinfo.c \
	ccifree.c ccibind.c cciunbind.c cciattach.c ccidetach.c cciswitch.c \
	msg.c

OBJS = $(SRCS:.c=.o)

MAINS = ccisrvinfo ccildlist cciload ccildinfo ccilinfo ccisubinfo ccifree \
	ccibind cciunbind cciattach ccidetach cciswitch 

.PRECIOUS: $(SRCS)

install: all dirs
	for i in $(MAINS)                                ;\
	do                                                \
		cpset $$i $(BIN)/$$i 0700 root sys       ;\
	done

all: $(MAINS)

dirs:
	-@[ -d $(ULIB) ] || mkdir $(ULIB)
	-@[ -d $(BIN) ]  || mkdir $(BIN)

$(MAINS):
	$(CC) -o $@ $? $(LDFLAGS) -lmb2 $(SHLIBS)

clean :
	rm -f $(OBJS)

clobber :
	rm -f $(OBJS) $(MAINS)
	
ccisrvinfo : ccisrvinfo.o msg.o 

ccisrvinfo.o : ccisrvinfo.c common.h cci.h msg.h  main.h

cciload : cciload.o msg.o

cciload.o : cciload.c common.h cci.h msg.h main.h 

ccildlist : ccildlist.o msg.o

ccildlist.o : ccildlist.c common.h cci.h msg.h main.h 

ccildinfo : ccildinfo.o msg.o

ccildinfo.o : ccildinfo.c common.h cci.h msg.h main.h 

ccilinfo : ccilinfo.o msg.o

ccilinfo.o : ccilinfo.c common.h cci.h msg.h main.h 

ccisubinfo : ccisubinfo.o msg.o

ccisubinfo.o : ccisubinfo.c common.h cci.h msg.h main.h 

ccifree : ccifree.o msg.o

ccifree.o : ccifree.c common.h cci.h msg.h main.h 

ccibind : ccibind.o msg.o

ccibind.o : ccibind.c common.h cci.h msg.h main.h 

cciunbind : cciunbind.o msg.o

cciunbind.o : cciunbind.c common.h cci.h msg.h main.h 

cciattach : cciattach.o msg.o

cciattach.o : cciattach.c common.h cci.h msg.h main.h 

ccidetach : ccidetach.o msg.o

ccidetach.o : ccidetach.c common.h cci.h msg.h main.h 

cciswitch : cciswitch.o msg.o

cciswitch.o : cciswitch.c common.h cci.h msg.h main.h 

msg.o : msg.c common.h cci.h msg.h  
