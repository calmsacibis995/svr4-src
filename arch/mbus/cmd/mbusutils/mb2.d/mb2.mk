#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/mb2.mk	1.3"

DASHO = -O
DIR  = $(ROOT)/usr/lbin
SBIN  = $(ROOT)/sbin
INC = $(ROOT)/usr/include
SHLIBS = -dy
LOCAL = -lmb2
LDFLAGS = -s $(IFLAG) $(LOCAL) 
CFLAGS = $(DASHO) -I$(INC)
STRIP = strip
SIZE = size
LIST = lp
MAKEFILE = mb2.mk

all: dir dirs

install:  all
	-@if [ $(BUS) = MB2 -o $(BUS) = MB2AT ];then			 \
	for i in *.d							;\
	do 								 \
		cd $$i							;\
		echo "====== mb2.d/$$i..."				;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	 \
		"ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" 			 \
		"MORECPP=$(MORECPP) -D$(BUS)" "BUS=$(BUS)" install	;\
		cd ..	 						;\
	done 								;\
	fi

dir:
	-@[ -d $(DIR) ] || mkdir $(DIR)
	-@[ -d $(SBIN) ] || mkdir $(SBIN)

dirs:
	-@if [ $(BUS) = MB2 -o $(BUS) = MB2AT ];then			 \
	for i in *.d							;\
	do								 \
		cd $$i							;\
		echo "====== mb2.d/$$i..."				;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	 \
		"ROOT=$(ROOT)" "SHLIBS=$(SHLIBS)" 	 		 \
		"MORECPP=$(MORECPP) -D$(BUS)" "BUS=$(BUS)" 		;\
		cd ..							;\
	done								;\
	fi

clean:
	-@for i in *.d; do cd $$i; make -f `basename $$i .d`.mk clean;cd ..;done
clobber:
	-@for i in *.d; do cd $$i; make -f `basename $$i .d`.mk clobber;cd ..;done
