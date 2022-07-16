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

#ident	"@(#)mbus:cmd/enet/Ina.d/Ina.mk	1.3.1.1"

# This makefile recreates the binaries from the respective hex dumps
# using the 'deodx' program that is used for a similar purpose in the
# layers component.
# Note that the bytes must be swapped due to the fact that od -x interprets
# 2 byte words, and the byte ordering on VAX and i386 is reversed from
# the character ordering.
# NOTE: if this is ported to a 3b2 or Amdahl, don't swap the bytes

DEODX = $(ROOT)/usr/src/cmd/layers/deodx

PCT = %

DIR = $(ROOT)/etc

INS = install

OBJS = ina961.36 ina961.37 ina961.36.M ina961.37.M

BINARIES = ina961.36 ina961.37 ina961.36.M ina961.37.M

BINSRCS = ina961.36src ina961.37src ina961.36.Msrc ina961.37.Msrc

INACFILE = ina961.data

.PRECIOUS: $(BINSRCS)

install: all
	for i in $(OBJS)                                 ;\
	do                                                \
		$(INS) -f $(DIR) -m 0700 -u root -g sys $$i      ;\
	done

	cp ina961.data.2 $(INACFILE)
	$(INS) -f $(DIR) -m 0700 -u root -g sys $(INACFILE)

all:	$(BINARIES)

$(DEODX):
	cd $(ROOT)/usr/src/cmd/layers; \
	$(MAKE) -f layers.mk deodx

$(BINARIES): $(DEODX)
	$(DEODX) <$@src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount $@src | \
	sed -e 's/.*= *//'` >$@
	
ina961.36 : ina961.36src

ina961.37 : ina961.37src

ina961.36.M : ina961.36.Msrc

ina961.37.M : ina961.37.Msrc

clean:

clobber:
	rm -f $(BINARIES) $(DEODX)
