#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)boot:boot/boot.mk	11.6.4.1"

#
# places .wboot and .fboot in $ROOT/etc
#
all clean clobber install:
# build common boot lib functions.
	cd bootlib						  ;\
	$(MAKE) -f bootlib.mk "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)"   \
	"STRIP=$(STRIP)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" $@
# next, arch specific boot
	if [ $(ARCH) = AT386 ];then					   \
		cd at386						  ;\
		$(MAKE) "BUS=$(BUS)" "ARCH=$(ARCH)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)"	   \
		"STRIP=$(STRIP)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" $@ ;\
	fi
#
# places .tboot and .dboot in $ROOT/etc
# note that the new disk layout for mbus systems uses both MSA and non-MSA
# bootloaders.
#
	if [ $(ARCH) = MBUS ];then				\
	if [ $(BUS) = MB1 ];then			   \
		cd mb1							  ;\
		$(MAKE) -f mb1.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "CC=$(CC)" "AS=$(AS)" \
		"LD=$(LD)" "STRIP=$(STRIP)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "ROOT=$(ROOT)" $@ ;cd ..;\
	fi ;\
	if [ $(BUS) = MB1 -o $(BUS) = MB2  ];then			   \
		cd msa							  ;\
		$(MAKE) -f msa.mk "BUS=$(BUS)" "ARCH=$(ARCH)" "CC=$(CC)" "AS=$(AS)" \
		"LD=$(LD)"	"STRIP=$(STRIP)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "ROOT=$(ROOT)" $@ ;cd ..;\
	fi ;\
	fi
