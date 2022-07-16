#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988, 1989  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:proto/i386/mbus/proto.mk	1.3.3.1"

PROTO   = $(ROOT)/usr/src/proto/i386/mbus
CMDS	= $(ROOT)/usr/src/proto/i386/mbus/cmd
TCPIO 	= $(PROTO)/distlist
TCONFIG = $(ROOT)/etc/default/bootserver/config.tape
KERNEL  = $(ROOT)/usr/src/proto/i386/kern-install/unix
PLIST	= $(ROOT)/opt/unix/Plist.dev $(ROOT)/opt/unix/Plist
DISTLIST	= $(ROOT)/opt/unix/Plist
DDISTLIST	= $(ROOT)/opt/unix/Plist.dev
BUS	= MB2
FSYS	= -Fs5
#FSYS	= 
RAMSIZE = 1400
RAMINODE = 240

tape:		boottape tapedist 
package:	boottape tapepackage 
addon:		rewind addpkg rewind
start:		$(PLIST) mkpkg

boottape:  setup start  proto.bld
	@echo "Insert BOOT/INSTALL tape and press <RETURN>: \c"; read a
	/usr/lib/tape/tapecntl -w
	if [ $(BUS) = MB1 ];then									 \
	$(ROOT)/sbin/sgib -T -N $(ROOT)/etc/boot					 \
					  -M $(ROOT)/etc/tsboot /dev/rmt/c0s0n 		;\
		echo "echo 0" >$(ROOT)/sbin/cpunum							;\
	else														 \
		$(ROOT)/sbin/sgib -T -M $(ROOT)/etc/tsboot /dev/rmt/c0s0n	;\
	fi
	dd if=$(TCONFIG) of=/dev/rmt/c0s0n bs=512 conv=sync			;\
	dd if=$(KERNEL) of=/dev/rmt/c0s0n bs=512  conv=sync
	> itmp
	cd $(ROOT);mkfs $(FSYS) $(PROTO)/itmp $(PROTO)/proto.bld 1 $(RAMSIZE)
	dd if=itmp of=iramd bs=$(RAMSIZE)k count=1 conv=sync
	dd if=iramd of=/dev/rmt/c0s0n ibs=1024 obs=512 count=$(RAMSIZE)
	rm -f itmp iramd proto.bld $(TCPIO)

mkfs: proto.bld
	> itmp
	cd $(ROOT);mkfs $(FSYS) $(PROTO)/itmp $(PROTO)/proto.bld 1 $(RAMSIZE)
#	rm -f proto.bld

proto.bld:
	rm -f proto.bld
	sed -e "s/RAMSIZE/`expr $(RAMSIZE) \* 2 `/" \
		-e "s:CMDS:$(CMDS):" \
		-e "s/RAMINODE/$(RAMINODE)/"  proto.tape >proto.bld

tapedist: cpio rewind

tapepackage:  cpio addpkg rewind


gettoend:
	-@/usr/lib/tape/tapecntl -p 1000 2>/dev/null  2>&1 |sync 
	sleep 5

cpio: $(PLIST) $(TCPIO)
	cd $(ROOT);  cat $(TCPIO)|cpio -omcC 102400 >/dev/rmt/c0s0n

rewind:
	/usr/lib/tape/tapecntl -w 
	sleep 20

addpkg:
	cd cmd; sh ./addpkg

mkpkg:
	cd cmd; sh ./mkpkg
	>mkpkg

$(TCPIO) distlist: 
	( cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $(DDISTLIST) | cut -f4 ) > $(TCPIO)
	( cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $(DISTLIST) | cut -f4 ) >> $(TCPIO)
	( cd $(ROOT); find var/spool/pkg -print) >> $(TCPIO)



pci:
	echo "Insert PCI_BOOT floppy and press <RETURN>: \c"; read a
	format -i 4 /dev/rdsk/f05ht
	$(ROOT)/sbin/sgib -c 40 -f 0 -r 2 -s 15 -d 512 -o 60 -i 3\
		-M$(ROOT)/etc/dsboot /dev/rdsk/f05ht
	cd $(ROOT);mkfs $(FSYS) /dev/dsk/f05hb $(PROTO)/proto.pci 1 9

debug:
	echo "Insert BOOT_DEBUG floppy and press <RETURN>: \c"; read a
	format -i 4 /dev/rdsk/f05ht
	cd $(ROOT);mkfs $(FSYS) /dev/dsk/f05ht $(PROTO)/proto.deb.flop 1 9

setup:   $(KERNEL)
	@echo "SETUP"
	cd cmd; make -f cmd.mk ROOT=$(ROOT) install 
	if [ $(BUS) = MB1 ];then \
		cp $(ROOT)/etc/dboot $(ROOT)/etc/boot ;\
		[ -d $(ROOT)/etc/default/bootserver ] || \
			mkdir $(ROOT)/etc/default/bootserver ;\
		cp $(ROOT)/usr/src/cmd/bootutils/config.d/config.tape \
		$(ROOT)/etc/default/bootserver ;\
	else \
		touch $(ROOT)/etc/boot ;\
	fi


$(PLIST):
	cd ../Plists;make -f Plists.mk ROOT=$(ROOT) BUS=$(BUS) PROTO=$(PROTO) install set

$(KERNEL):
	cd ../kern-install;make -f kernel.mk ROOT=$(ROOT) BUS=$(BUS) setup config unix

clean:
	cd $(PROTO);rm -f iramd itmp  $(TCPIO)
	cd ../kern-install;make -f kernel.mk ROOT=$(ROOT) BUS=$(BUS) clean
	cd ../Plists;make -ef Plists.mk ROOT=$(ROOT) BUS=$(BUS) clean
	cd cmd;make -ef cmd.mk ROOT=$(ROOT) BUS=$(BUS) clean

clobber: clean
	cd $(PROTO); rm -f setup proto.bld
	cd ../kern-install;make -f kernel.mk ROOT=$(ROOT) BUS=$(BUS) clobber
	cd ../Plists;make -ef Plists.mk ROOT=$(ROOT) BUS=$(BUS) clobber
	cd cmd;make -ef cmd.mk ROOT=$(ROOT) BUS=$(BUS) clobber
