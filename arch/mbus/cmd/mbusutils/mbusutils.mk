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

#ident	"@(#)mbus:cmd/mbusutils/mbusutils.mk	1.3"

DASHO = -O
DIR  = $(ROOT)/usr/lbin
SBIN  = $(ROOT)/sbin
INC = $(ROOT)/usr/include
ROOTLIBS = -dn
SHLIBS =
LDFLAGS = -s $(IFLAG) 
CFLAGS = $(DASHO) -I$(INC)
STRIP = strip
SIZE = size
LIST = lp
MAKEFILE = mbusutils.mk
MORECPP	= -D$(BUS)


all: dir dirs

install:  all
	-@for i in *.d							;\
	do 								 \
		[ $(BUS) != MB1 -a $$i = mb1.d ]     && continue		;\
		[ $(BUS) != MB2 -a $$i = mb2.d ]     && continue		;\
		[ $(BUS) != MB2AT -a $$i = mb2at.d ]   && continue		;\
		[ $(BUS) != AT386 -a $$i = at386.d ] && continue		;\
		cd $$i							;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	 \
		"ROOT=$(ROOT)" "ROOTLIBS=$(ROOTLIBS)" "SHLIBS=$(SHLIBS)" \
		"MORECPP=$(MORECPP) -D$(BUS)" "BUS=$(BUS)" install	;\
		cd ..	 						;\
	done

dir:
	-@[ -d $(DIR) ] || mkdir $(DIR)
	-@[ -d $(SBIN) ] || mkdir $(SBIN)

dirs:
	-@for i in *.d											;\
	do														 \
		cd $$i												;\
		echo "$$i..."										;\
		make -f `basename $$i .d`.mk "LDFLAGS=$(LDFLAGS)" 	\
		"ROOT=$(ROOT)" "ROOTLIBS=$(ROOTLIBS)" "SHLIBS=$(SHLIBS)" \
		"MORECPP=$(MORECPP) -D$(BUS)" "BUS=$(BUS)"	 	;\
		cd ..												;\
	done

clean:
	-@for i in *.d; do cd $$i; make -f `basename $$i .d`.mk clean;cd ..;done
clobber:
	-@for i in *.d; do cd $$i; make -f `basename $$i .d`.mk clobber;cd ..;done
