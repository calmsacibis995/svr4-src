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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/cci.mk	1.3"

#	Makefile for ibin directory

DIR  = $(ROOT)/usr/ibin
ETC  = $(ROOT)/etc
INC = $(ROOT)/usr/include
SHLIBS = -dy
LDFLAGS = -s $(IFLAG) -YU,$(ROOT)/usr/lib
CFLAGS = -O -I$(INC)
STRIP = strip
SIZE = size
LIST = lp
MAKEFILE = cci.mk
MORECPP	= -D$(BUS)


all: 
	-@for i in *.d							;\
	do								 \
		cd $$i							;\
		echo "$$i..."						;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	 \
		"ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" 			 \
		"MORECPP=$(MORECPP)" 					;\
		cd ..							;\
	done

install: all 
	-@for i in *.d							;\
	do								 \
		cd $$i							;\
		echo "$$i..."						;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	 \
		"ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" 			 \
		"MORECPP=$(MORECPP)" install 				;\
		cd ..							;\
	done

clean:
	-@for i in *.d;do cd $$i;make -f `basename $$i .d`.mk clean;cd ..;done

clobber:
	-@for i in *.d;do cd $$i;make -f `basename $$i .d`.mk clobber;cd ..;done
