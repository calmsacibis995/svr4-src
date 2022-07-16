#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto:i386/at386/tape.mk	1.4.1.1"

ROOT=/
STRIP=strip
IRQ=5
ADDR=300
INC=$(ROOT)/usr/include

default:
	echo "Nothing specified"

all:
	-make -f proto.mk ROOT=$(ROOT) devs install shlibs

package: bootflop

newdev:
	rm -f drive_info FLOPPY/.floppydate LABEL

bootflop: tape/unix drive_info LABEL 
	$(CC) -O -s -o tape/machid -DAT386=1 -I$(INC) ../machid.c -dn
	echo $(ROOT)/usr/src/proto/i386/at386/tape/machid | sh ../prep NO_UNIX
	sh cmd/boot_make.sh tape/proto.tape tape/proto.T

drive_info: ask_drive
	sh ask_drive 2>drive_info

ask_drive:
	cd cmd; make -f cmd.mk $@

LABEL:
	make -f proto.mk LABEL

tape/unix:	qt tape.idins
	sh ./tape.idins -d qt
	sh ./mini_kernel on
	sh ./tape.idins -a qt
	-$(ROOT)/etc/conf/bin/idbuild -DRAMD_BOOT
	sh ./tape.idins -d qt
	sh ./mini_kernel off
	cp $(ROOT)/etc/conf/cf.d/unix tape/unix
	echo $(ROOT)/usr/src/proto/i386/at386/tape/unix | sh ../prep NO_UNIX
	$(STRIP) tape/unix

qt: Master Space.c System Driver.o

Space.c Driver.o Master: Size

Size: pkg.qt
	(ADDR2=`expr $(ADDR) + 1` ;\
	sed -e "s/INT	LADD	HADD/$(IRQ)	$(ADDR)	$$ADDR2/g" < System > S ;\
	cat S > System )
	sed -e 's/0	0	2/4	0	2/g' Master > M
	cat M > Master
	echo $(ROOT)/usr/src/proto/i386/at386/tapecntl | sh ../prep NO_UNIX
	-$(STRIP) tapecntl
	rm -f M S

pkg.qt: always
	cp $(ROOT)/usr/src/pkg/qt/*tapecntl* .
	cp $(ROOT)/usr/src/pkg/qt/*Space.c* .
	cp $(ROOT)/usr/src/pkg/qt/*Driver.o* .
	cp $(ROOT)/usr/src/pkg/qt/*Master* .
	cp $(ROOT)/usr/src/pkg/qt/*System* .

always:

tape.idins:
	cp tape/$@ $@

clean:
	-rm -f tape.idins tape/machid tapecntl
	-rm -f Master Space.c System Driver.o tape/unix

clobber: clean
	- make -f proto.mk clobber
