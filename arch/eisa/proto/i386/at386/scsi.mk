#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)eisa:proto/i386/at386/scsi.mk	1.3.1.1"

ROOT=/
STRIP=strip
SBUS=wd7000
INC=$(ROOT)/usr/include

default:
	echo "Nothing specified"

all:
	-make -f proto.mk ROOT=$(ROOT) devs install shlibs

newdev:
	rm -f drive_info FLOPPY/.floppydate LABEL

package: bootflop

bootflop: scsi/unix drive_info LABEL
	$(CC) -O -s -o scsi/machid -DAT386=1 -DSCSI -I$(INC) ../machid.c -dn
	echo $(ROOT)/usr/src/proto/i386/at386/scsi/machid | sh ../prep NO_UNIX
	sh scsi/boot_make scsi/proto.scsi scsi/proto.S $(SBUS)

drive_info: ask_drive
	sh ask_drive 2>drive_info

ask_drive:
	cd cmd; make -f cmd.mk $@

LABEL:
	make -f proto.mk LABEL

scsi/unix:	scsi tape.idins
	-mv $(ROOT)/etc/conf/cf.d/mdevice.scsi $(ROOT)/etc/conf/cf.d/mdevice > /dev/null 2>&1
	-mv $(ROOT)/etc/conf/sdevice.d/.hd $(ROOT)/etc/conf/sdevice.d/hd > /dev/null 2>&1
	-mv $(ROOT)/etc/conf/cf.d/sassign.scsi $(ROOT)/etc/conf/cf.d/sassign > /dev/null 2>&1
	sh ./mini_kernel on
	grep -v "^hd	" $(ROOT)/etc/conf/cf.d/mdevice > M
	mv $(ROOT)/etc/conf/cf.d/mdevice $(ROOT)/etc/conf/cf.d/mdevice.scsi
	mv M $(ROOT)/etc/conf/cf.d/mdevice
	mv $(ROOT)/etc/conf/sdevice.d/hd $(ROOT)/etc/conf/sdevice.d/.hd
	sed -e 's/hd/sd01/g' < $(ROOT)/etc/conf/cf.d/sassign > M
	mv $(ROOT)/etc/conf/cf.d/sassign $(ROOT)/etc/conf/cf.d/sassign.scsi
	mv M $(ROOT)/etc/conf/cf.d/sassign
	- cd scsi ;\
	if [ $(SBUS) = ad1542 ] ;\
	then sed -e 's/INT	IOA	IOE/11	330	333/g' System > S;\
	elif [ $(SBUS) = esc ] ;\
	then sed -e 's/INT	IOA	IOE/11	0	0/g' System > S;\
	else sed -e 's/INT	IOA	IOE/15	350	353/g' System > S;\
	fi ;\
	cat S > System;\
	rm -f S
	- cd scsi ;\
	if [ $(SBUS) = ad1542 ] ;\
	then sed -e 's/0	0	1	2	DMA/0	4	1	2	5/g' Master > M ;\
	elif [ $(SBUS) = esc ] ;\
	then sed -e 's/0	0	1	2	DMA/0	4	1	2	-1/g' Master > M ;\
	else sed -e 's/0	0	1	1	DMA/0	4	1	1	6/g' Master > M ;\
	fi ;\
	cat M > Master;\
	rm -f M 
	cd st01 ;\
	sed -e 's/0	0	0/0	10	0/g' Master > M ;\
	cat M > Master;\
	rm -f M 
	cd sd01 ;\
	sed -e 's/0-6	0-6	0/35-41	35-41	0/g' Master > M ;\
	cat M > Master;\
	rm -f M 
	for i in sd01 scsi st01 ;\
	do \
	cd $$i ;\
	sh ../tape.idins -d $$i ;\
	sh ../tape.idins -a $$i ;\
	cd .. ;\
	done
	cd . ; \
	if [ $(SBUS) = esc ] ; \
	then $(ROOT)/etc/conf/bin/idbuild -DRAMD_BOOT -DEISA -DEVC ;\
	else $(ROOT)/etc/conf/bin/idbuild -DRAMD_BOOT ;\
	fi
	for i in sd01 scsi st01 ;\
	do \
	sh ./tape.idins -d $$i ;\
	done
	mv $(ROOT)/etc/conf/cf.d/mdevice.scsi $(ROOT)/etc/conf/cf.d/mdevice
	mv $(ROOT)/etc/conf/sdevice.d/.hd $(ROOT)/etc/conf/sdevice.d/hd
	mv $(ROOT)/etc/conf/cf.d/sassign.scsi $(ROOT)/etc/conf/cf.d/sassign
	sh ./mini_kernel off
	cp $(ROOT)/etc/conf/cf.d/unix scsi/unix
	echo $(ROOT)/usr/src/proto/i386/at386/scsi/unix | sh ../prep NO_UNIX
	$(STRIP) scsi/unix

scsi: stapecntl st01/Master sd01/Master st01/Driver.o sd01/Driver.o scsi/Master scsi/Driver.o $(ROOT)/usr/include/sys/sdi.h DISK

st01/Master st01/Driver.o: $(ROOT)/usr/src/pkg/scsi.in.src/st01/Driver.o
	-mkdir st01
	cp $(ROOT)/usr/src/pkg/scsi.in.src/st01/* st01

sd01/Master sd01/Driver.o: $(ROOT)/usr/src/pkg/scsi.src/sd01/Driver.o
	-mkdir sd01
	cp $(ROOT)/usr/src/pkg/scsi.src/sd01/* sd01
	mv sd01/space.c sd01/Space.c

scsi/Master scsi/Driver.o: $(ROOT)/usr/src/pkg/scsi.src/scsi/Driver.o $(ROOT)/usr/src/pkg/scsi.in.src/scsi/Driver.o
	-mkdir scsi sys
	-if [ $(SBUS) = ad1542 ] ;\
	then echo "Adaptec AHA-1542A Driver" ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/scsi/* scsi ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/scsi.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/ad1542.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/sdi.h sys ;\
	elif [ $(SBUS) = esc ] ;\
	then echo "Olivetti ESC-1 Driver" ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/scsi/* scsi ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/scsi.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/esc.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/sdi.h sys ;\
	else echo "Western Digital Driver" ;\
	     cp $(ROOT)/usr/src/pkg/scsi.src/scsi/* scsi ;\
	     cp $(ROOT)/usr/src/pkg/scsi.src/sys/scsi.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.src/sys/sdi.h sys ;\
	     cp $(ROOT)/usr/src/pkg/scsi.src/sys/had.h sys ;\
	     mv scsi/space.c scsi/Space.c ;\
	fi


DISK: $(ROOT)/usr/src/pkg/scsi.src/cmd/DISK $(ROOT)/usr/src/pkg/scsi.src/cmd/tc.index
	cp $(ROOT)/usr/src/pkg/scsi.src/cmd/format.d/* .
	cp $(ROOT)/usr/src/pkg/scsi.src/cmd/tc.index .
	cp $(ROOT)/usr/src/pkg/scsi.src/cmd/DISK .
	echo $(ROOT)/usr/src/proto/i386/at386/DISK | sh ../prep NO_UNIX
	-$(STRIP) DISK

stapecntl:
	cp $(ROOT)/usr/src/pkg/scsi.in.src/cmd/tapecntl stapecntl
	echo $(ROOT)/usr/src/proto/i386/at386/stapecntl | sh ../prep NO_UNIX
	-$(STRIP) stapecntl

$(ROOT)/usr/include/sys/sdi.h: $(ROOT)/usr/src/pkg/scsi.src/sys $(ROOT)/usr/src/pkg/scsi.in.src/sys sys/scsi.h sys/sdi.h
	-mkdir sys
	cp $(ROOT)/usr/src/pkg/scsi.src/sys/sd01.h sys
	cp $(ROOT)/usr/src/pkg/scsi.in.src/sys/st01.h sys
	cp sys/*.h $(ROOT)/usr/include/sys

always:

tape.idins:
	cp tape/$@ $@

clean:
	-rm -f tape.idins scsi/machid
	-rm -f Size etc stapecntl scsiformat DISK tc.index sd00.0 sd01.1
	-rm -fr sd01 st00 st01 scsi/Master scsi/System scsi/Driver.o scsi/Space.c scsi/space.c scsi/Driver2.o scsi/Driver3.o scsi/System2 scsi/System3
	-rm -fr sys cmd/format.d cmd/mkdev.d qtape1 scsimkdev tc.mkdev
	-rm -rf scsi/unix
	cd $(ROOT)/usr/include/sys ; rm -f had.h scsi.h sd01.h sdi.h st01.h ad1542.h

clobber: clean
	- make -f proto.mk clobber
