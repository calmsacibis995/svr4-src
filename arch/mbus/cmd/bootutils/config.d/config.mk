#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Reserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/bootutils/config.d/config.mk	1.3.1.1"

# This makefile recreates the pci files from pci.src using the 'deodx'
# program that is used for a similar purpose in the layers component.
# Note that the bytes must be swapped due to the fact that od -x interprets
# 2 byte words, and the byte ordering on VAX and i386 is reversed from
# the character ordering.
# NOTE: if this is ported to a 3b2 or Amdahl, don't swap the bytes


ETC	=	 $(ROOT)/etc
BOOTCFG = $(ETC)/default/bootserver
MAKEFILE =	 config.mk
TAPEBOOT = config.tape
SCHOONER = config.520
SYPMDP = config.mdp
DEFCONFIG = config
PCIDEF   = pci258
PCISMALL = pci258.1m

STANDCFG = $(ROOT)/stand
SCHOONER_STAND = stand.cfg.520
SYPMDP_STAND = stand.cfg.mdp

DEODX = $(ROOT)/usr/src/cmd/layers/deodx
BINARIES = pci258 pci258.1m
BINSRCS = pci258src pci258.1msrc
.PRECIOUS: $(SRCS) $(BINSRCS)

all:  $(DEODX) $(BINARIES)

install:  
	if [ $(BUS) = MB2 ];then					 \
		[ -d $(ETC)/default ] || mkdir $(ETC)/default		;\
		[ -d $(BOOTCFG) ]     || mkdir  $(BOOTCFG)		;\
		[ -d $(STANDCFG) ]     || mkdir  $(STANDCFG)		;\
		cp $(SCHOONER) $(DEFCONFIG)				;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(TAPEBOOT) ;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(SCHOONER) ;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(SYPMDP) ;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(PCIDEF) ;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(PCISMALL) ;\
		install -f $(BOOTCFG) -m 744 -u root -g bin $(DEFCONFIG) ;\
		cp $(SCHOONER_STAND) $(DEFCONFIG)				;\
		install -f $(STANDCFG) -m 744 -u root -g bin $(PCIDEF) ;\
		install -f $(STANDCFG) -m 744 -u root -g bin $(PCISMALL) ;\
		install -f $(STANDCFG) -m 744 -u root -g bin $(DEFCONFIG) ;\
		rm $(DEFCONFIG)											  ;\
	fi

$(DEODX):
	cd $(ROOT)/usr/src/cmd/layers; \
	$(MAKE) -f layers.mk deodx

$(BINARIES): $(DEODX)
	$(DEODX) <$@src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount $@src | \
	sed -e 's/.*= *//'` >$@

clean:

clobber: clean
	rm -f $(BINARIES) $(DEODX)
