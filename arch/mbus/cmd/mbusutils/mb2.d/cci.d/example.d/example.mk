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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/example.d/example.mk	1.3"

ULIB = $(ROOT)/usr/lib

SDIR = $(ULIB)/cci

BIN = $(SDIR)/example

INC = $(ROOT)/usr/include

OBJS = example.csd example.lit example.p86

.PRECIOUS: $(OBJS)

install: dirs
	for i in $(OBJS)                                 ;\
	do                                                \
		cpset $$i $(BIN)/$$i 0700 root sys       ;\
	done

dirs:
	-@[ -d $(ULIB) ] || mkdir $(ULIB)
	-@[ -d $(SDIR) ] || mkdir $(SDIR)
	-@[ -d $(BIN) ]  || mkdir $(BIN)

clean:

clobber:
