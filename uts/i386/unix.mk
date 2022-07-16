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

#ident	"@(#)kern-mk:unix.mk	1.2.1.2"

NET=
FRC =
ROOT = $(ROOT)
CC = cc
AS = as
LD = ld
CPP = ${CCSROOT}/usr/ccs/lib/${PFX}cpp
STRIP = strip
#       The default is AT386 (BUS=AT386), all drivers, no debugger
SYSTEM = system.std
BUS = AT386
ARCH=AT386
NET2=
NET3=
MORECPP = -D$(BUS) -D$(ARCH) -DWEITEK -DWEITEK_EMULATOR
CONF = $(ROOT)/etc/conf

INCRT = $(ROOT)/usr/src/uts/i386
CPPFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP)
CFLAGS = -O $(CPPFLAGS)
MKUNIX = $(ROOT)/etc/conf/bin/idbuild
BASE = master machine exec system vmsys vpix filsys drivers disp kdb bootstrap emulator vuifile kernmap
NETW =  des inet klm rpc ktli

all: $(BASE) $(NETW) mkunix

nonet: $(BASE) mkunix

mkunix:
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		cp kernmap temp.c ; \
		$(CPP) $(CPPFLAGS) -P temp.c temp.i ; \
		cp temp.i $(CONF)/cf.d/kernmap ; \
	else \
		cp vuifile temp.c ; \
		$(CC) $(CPPFLAGS) -P temp.c ; \
		cp temp.i $(CONF)/cf.d/vuifile ; \
	fi
	@rm -f temp.c temp.i
	CC="$(CC)" CFLAGS="$(CFLAGS)" AS="$(AS)" LD="$(LD)" STRIP="$(STRIP)" FRC="$(FRC)" MORECPP="$(MORECPP)" INCRT="$(INCRT)" $(MKUNIX)

master:FRC
	cd master.d; $(MAKE) -f master.d.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" 

vpix:FRC
	cd vx; $(MAKE) -f vx.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

machine:FRC
	cd ml; $(MAKE) -f ml.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "CPP=$(CPP)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

exec:FRC
	cd exec; $(MAKE) -f exec.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

system:FRC
	cd os; $(MAKE) -f os.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

vmsys:FRC
	cd vm; $(MAKE) -f vm.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

kdb:FRC
	cd kdb; $(MAKE) -f kdb.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"


bootstrap:FRC
	cd boot; $(MAKE) -f boot.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)" "STRIP=$(STRIP)" install

filsys:FRC
	cd fs; $(MAKE) -f fs.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

disp:FRC
	cd disp; $(MAKE) -f disp.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "CONF=$(CONF)"

drivers:FRC
	cd io; $(MAKE) -f io.mk ${NONETWORK} "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

inet:FRC
	cd netinet; $(MAKE) -f netinet.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

klm:FRC
	cd klm; $(MAKE) -f klm.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

rpc:FRC
	cd rpc; $(MAKE) -f rpc.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

ktli:FRC
	cd ktli; $(MAKE) -f ktli.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

des:FRC
	cd des; $(MAKE) -f des.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)"

emulator:FRC
	cd fp; $(MAKE) -f fp.mk "ROOT=$(ROOT)" "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "CPP=$(CPP)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" "INCRT=$(INCRT)" "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" install

clean:
	cd master.d; $(MAKE) -f master.d.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd vx; $(MAKE) -f vx.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd ml; $(MAKE) -f ml.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd exec; $(MAKE) -f exec.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd os; $(MAKE) -f os.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd vm; $(MAKE) -f vm.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd boot; $(MAKE) -f boot.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd fs; $(MAKE) -f fs.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd io; $(MAKE) -f io.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd disp; $(MAKE) -f disp.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd fp; $(MAKE) -f fp.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd kdb; $(MAKE) -f kdb.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd des; $(MAKE) -f des.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd klm; $(MAKE) -f klm.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd ktli; $(MAKE) -f ktli.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd netinet; $(MAKE) -f netinet.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd rpc; $(MAKE) -f rpc.mk clean "ROOT=$(ROOT)" "CONF=$(CONF)"

clobber:
	cd master.d; $(MAKE) -f master.d.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)" "NET=$(NET)" "BUS=$(BUS)"  "ARCH=$(ARCH)" "NET2=$(NET2)" "NET3=$(NET3)"
	cd vx; $(MAKE) -f vx.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd ml; $(MAKE) -f ml.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd exec; $(MAKE) -f exec.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd os; $(MAKE) -f os.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd vm; $(MAKE) -f vm.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd boot; $(MAKE) -f boot.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd fs; $(MAKE) -f fs.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd io; $(MAKE) -f io.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)" "NET=$(NET)" "BUS=$(BUS)" "ARCH=$(ARCH)" "NET2=$(NET2)" "NET3=$(NET3)"
	cd disp; $(MAKE) -f disp.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd fp; $(MAKE) -f fp.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd kdb; $(MAKE) -f kdb.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd des; $(MAKE) -f des.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd klm; $(MAKE) -f klm.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd ktli; $(MAKE) -f ktli.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd netinet; $(MAKE) -f netinet.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	cd rpc; $(MAKE) -f rpc.mk clobber "ROOT=$(ROOT)" "CONF=$(CONF)"
	rm -f $(CONF)/cf.d/vuifile

FRC:
