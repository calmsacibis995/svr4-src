#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto:i386/at386/proto.mk	1.3.5.2"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

ROOT=/
PLIST=USO
INC=$(ROOT)/usr/include
PCNT=%
VER=4.0
STRIP	= strip
MAKE	= make
# BASEFLOPS=8
# BLKS=2370

#       Makefile for installation diskettes 

TEXTS = proto.flop proto.mk \
	inittab inittab2 ioctl.syscon

TEXTFILES = group passwd emulator

BOOTS = boot fboot

DFLT_BOOTS= default.att default.cpq default.at386 default.att512 workstations 

ETCBINARIES = nothing_yet

USRBIN = chmod chgrp chown find cpio mkdir expr stty mv rm dd ed sleep

USRLIB = libc.so.1

USRSBIN = machine_type

FSCK_SP = fsck.ufs

FSS5 = labelit.s5 mkfs.s5 mount.s5 fsck.s5

FSBFS = fsck.bfs mkfs.bfs mount.bfs

FSUFS = labelit.ufs mount.ufs mfsk.ufs

SBIN = fdisk flop_disk disksetup uadmin autopush chkconsole sh sync umount mknod mount wsinit partsize memsize init

AT386PROGS = $(ROOT)/etc/inittab3 $(INSTALLSCRIPTS)

INSTALLSCRIPTS = $(ROOT)/tmp/INSTALL3 $(ROOT)/tmp/Plist.dev $(ROOT)/etc/conf/cf.d/mdevice.base

ALL = $(TEXTS) $(BOOTS) LABEL TIMEZONE ask_drive boot_make $(USRBIN) $(USRSBIN) $(FSS5) $(FSUFS) $(FSBFS) $(FSCK_SP) $(SBIN) chan.ap $(USRLIB) $(TEXTFILES) $(DFLT_BOOTS) att at386 compaq 

first links:
	cd cmd; make -f cmd.mk install; cd ..
	-cd ../PACKAGES/peruser; make -f peruser.mk ../../initial; \
		cd ../../at386
	-cp ../initial initial
	-echo usr/src/proto/i386/at386/initial | sh ../prep NO_UNIX
	-$(STRIP) initial \
		$(ROOT)/usr/bin/spline \
		$(ROOT)/usr/bin/tabs $(ROOT)/usr/bin/units \
		$(ROOT)/usr/bin/pwd $(ROOT)/usr/bin/x286emul
	cp ../Plists/Plist.dev Plist.dev

all: $(ALL)

install: first $(AT386PROGS) prep perms all $(ROOT)/var/sadm/install/contents

bootflop:
	echo "Please run tape.mk or scsi.mk"

package: install dist

$(ROOT)/stand/unix: $(ROOT)/stand $(ROOT)/etc/conf/cf.d/unix
	cp $(ROOT)/etc/conf/cf.d/unix $(ROOT)/stand/unix
	echo stand/unix | sh ../prep NO_UNIX

$(ROOT)/stand:
	mkdir $(ROOT)/stand

dist:	LABEL make_flops drive_info
	chmod +x make_flops
	@echo "You will need `sed -n '1s/.* //p' LABEL` floppies to continue."
	( PATH=$$PATH:`pwd` ; H=`pwd`; cd $(ROOT)/; make_flops $$H ) 
	@echo "DONE"

LABEL: FLOPPY/.floppydate
	( BASEFLOPS=`ls FLOPPY/cpiolist?? | wc -l` ; \
	  BASEFLOPS=`expr $${BASEFLOPS} + 2` ; \
	  LOC=`cut -f2 drive_info` ; \
	  echo "$(VER).3 386unix Fnd Set 1 of $${BASEFLOPS}\n$${LOC}" > LABEL \
	)

FLOPPY/.floppydate: ../Plists/Plist drive_info
	-mkdir FLOPPY
	rm -f FLOPPY/cpiolist??
	(A=`pwd`; BLKS=`cut -f3 drive_info` ; \
		cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $$A/../Plists/Plist $$A/../Plists/Plist.$(PLIST) | cut -f4 | \
		etc/mkflist - $$A/FLOPPY/cpiolist $$A/FLOPPY/err $$BLKS 3 ; \
	)
	- (A=`pwd`; ST=`expr \`ls -r FLOPPY/cpiolis* | sed -n 1p | cut -c16-\` + 1`; BLKS=`cut -f3 drive_info` ; \
		cd $(ROOT); \
		if [ -d usr/src/pkg/scsi.in.src ] ;\
		then find usr/src/pkg/scsi.in.src usr/src/pkg/scsi.src -print | \
		etc/mkflist - $$A/FLOPPY/cpiolist $$A/FLOPPY/err $$BLKS $$ST ; \
		fi ; \
	)
	touch $@

drive_info: ask_drive
	sh ask_drive 2>drive_info

ask_drive boot_make:
	cd cmd; make -f cmd.mk $@; cd ..

make_flops: ALWAYS
	cp cmd/make_flops.sh make_flops

ALWAYS:
	@: echo perform always

$(ETCBINARIES): $(ROOT)/etc/$@
	$(STRIP) $(ROOT)/etc/$@
	echo etc/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/etc/$@ . 

boot: $(ROOT)/etc/.wboot
	- ln $(ROOT)/etc/.wboot $(ROOT)/etc/boot
	- $(STRIP) $(ROOT)/etc/boot
	echo etc/boot | sh ../prep NO_UNIX
	cp $(ROOT)/etc/boot .

fboot: $(ROOT)/etc/.fboot
	- ln $(ROOT)/etc/.fboot $(ROOT)/etc/fboot
	- $(STRIP) $(ROOT)/etc/fboot
	echo etc/fboot | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fboot .

$(TEXTFILES):
	echo etc/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/etc/$@ .

DISTLIST:
	( cd $(ROOT)				;\
	rm -f $(ROOT)/etc/mnttab                ;\
	for i in `echo * usr/*`			;\
	do if [ "$$i" != usr -a "$$i" != usr/src -a "$$i" != i386 ]	;\
		then find "$$i" -print ; fi	;\
	done ) >DISTLIST

$(ROOT)/etc/.installdate:
	date +$(PCNT)m$(PCNT)d0001 > $(ROOT)/etc/.installdate
	date +$(PCNT)m$(PCNT)d0000 > $(ROOT)/etc/.installstart
	touch `cat $(ROOT)/etc/.installdate` $@
	touch `cat $(ROOT)/etc/.installstart` $(ROOT)/etc/.installstart
	@echo "ISSUE DATE IS: `ls -lgo $(ROOT)/etc/.installdate | cut -c24-`"

newissue:
	rm -f $(ROOT)/etc/.installdate $(ROOT)/etc/.installstart

perms:	../Plists/Plist
	-(A=`pwd`; cd $(ROOT); etc/setmods $$A/../Plists/Plist)
	-(A=`pwd`; cd $(ROOT); etc/setmods $$A/../Plists/Plist.$(PLIST))

prep:   $(ROOT)/etc/.installdate
#	sh ../prep NO_UNIX < DISTLIST
	@echo "Prep-ing Plist: `date`"
	sed -e "/^$$/d" -e "/^#/d" ../Plists/Plist ../Plists/Plist.$(PLIST) | cut -f4 | \
		egrep -v 'usr/bin/ar|usr/bin/idas|usr/bin/idld|usr/bin/idcc|usr/lib/idacomp|usr/lib/idcpp' | \
		egrep -v 'usr/bin/mcs' | \
		egrep -v 'usr/bin/what' | \
		sh ../prep NO_UNIX
	@echo "Pstamping complete: `date`"
	@echo "\nSetting distribution files to official issue date."
	@echo "ISSUE DATE IS: `ls -lgo $(ROOT)/etc/.installdate | cut -c24-`"
	@echo "Press <ENTER> when ready to proceed. \c"; read okidoki
	( A=`pwd`; cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $$A/../Plists/Plist $$A/../Plists/Plist.$(PLIST) | cut -f4 | \
		egrep -v 'etc/.installstart' | \
		xargs touch -c `cat $(ROOT)/etc/.installdate` \
	)
	@echo "Distribution files have been set to Official Date of Issue."
	rm -f $(ROOT)/var/sadm/install/contents
	(A=`pwd`; cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $$A/../Plists/Plist $$A/../Plists/Plist.$(PLIST) | etc/contents > $(ROOT)/var/sadm/install/contents ; \
	)

TIMEZONE: $(ROOT)/etc/TIMEZONE
	cp $? .

$(ROOT)/etc/inittab3: $(ROOT)/etc/inittab
	cp $(ROOT)/etc/inittab $(ROOT)/etc/inittab3

$(ROOT)/tmp/INSTALL3: INSTALL3
	install -f $(ROOT)/tmp -m 555 -u bin -g bin INSTALL3

INSTALL3: cmd/INSTALL3
	cp cmd/INSTALL3 INSTALL3

$(ROOT)/etc/conf/cf.d/mdevice.base: $(ROOT)/etc/conf/cf.d/mdevice
	-grep "	etc/conf/sdevice.d/" ../Plists/Plist | cut -d/ -f4 | while read i; do grep -n "^$$i	" $(ROOT)/etc/conf/cf.d/mdevice; done > MLIST
	sort -n MLIST | cut -f2- -d: > $(ROOT)/etc/conf/cf.d/mdevice.base
	rm -f MLIST

$(ROOT)/tmp/Plist.dev: Plist.dev
	install -f $(ROOT)/tmp -m 555 -u bin -g bin Plist.dev

$(ROOT)/var/sadm/install/contents: ../Plists/Plist
	(A=`pwd`; cd $(ROOT); \
		sed -e "/^$$/d" -e "/^#/d" $$A/../Plists/Plist $$A/../Plists/Plist.$(PLIST) | etc/contents > $(ROOT)/var/sadm/install/contents ; \
	)

$(DFLT_BOOTS): $(ROOT)/etc/default/default.att $(ROOT)/etc/default/default.cpq \
	$(ROOT)/etc/default/default.at386 $(ROOT)/etc/default/default.att512 \
	$(ROOT)/etc/default/workstations 
	cp $(ROOT)/etc/default/$@ $@

$(USRBIN): $(ROOT)/usr/bin/$@
	$(STRIP) $(ROOT)/usr/bin/$@
	echo usr/bin/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/usr/bin/$@ $@

$(USRLIB): $(ROOT)/usr/lib/$@
	echo usr/lib/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/usr/lib/$@ $@

$(USRSBIN): $(ROOT)/usr/sbin/$@
	$(STRIP) $(ROOT)/usr/sbin/$@
	echo usr/sbin/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/usr/sbin/$@ $@

$(SBIN): $(ROOT)/sbin/$@
	$(STRIP) $(ROOT)/sbin/$@
	echo sbin/$@ | sh ../prep NO_UNIX
	cp $(ROOT)/sbin/$@ $@

$(FSS5):
	$(STRIP) $(ROOT)/etc/fs/s5/mount
	echo etc/fs/s5/mount | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/s5/mount mount.s5
	$(STRIP) $(ROOT)/etc/fs/s5/mkfs
	echo etc/fs/s5/mkfs | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/s5/mkfs mkfs.s5
	$(STRIP) $(ROOT)/usr/lib/fs/s5/labelit
	echo usr/lib/fs/s5/labelit | sh ../prep NO_UNIX
	cp $(ROOT)/usr/lib/fs/s5/labelit labelit.s5
	$(STRIP) $(ROOT)/etc/fs/s5/fsck
	echo etc/fs/s5/fsck | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/s5/fsck fsck.s5

$(FSUFS):
	$(STRIP) $(ROOT)/etc/fs/ufs/mount
	echo etc/fs/ufs/mount | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/ufs/mount mount.ufs
	$(STRIP) $(ROOT)/usr/lib/fs/ufs/labelit
	echo usr/lib/fs/ufs/labelit | sh ../prep NO_UNIX
	cp $(ROOT)/usr/lib/fs/ufs/labelit labelit.ufs
	$(STRIP) $(ROOT)/etc/fs/ufs/mkfs
	echo etc/fs/ufs/mkfs | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/ufs/mkfs mkfs.ufs

mkfs.ufs.dy:
	( cd $(ROOT)/usr/src/cmd/fs.d/ufs/mkfs;\
	$(MAKE) -f mkfs.mk mkfs ROOTLIBS='-dy' ;\
	mv mkfs $(ROOT)/usr/src/proto/i386/at386/mkfs.ufs;\
	$(MAKE) -f mkfs.mk clobber )
	$(STRIP) mkfs.ufs
	echo usr/src/proto/i386/at386/mkfs.ufs | sh ../prep NO_UNIX

fsck.ufs:
	( cd $(ROOT)/usr/src/cmd/fs.d/ufs/fsck;\
	$(MAKE) -f fsck.mk fsck ROOTLIBS='-dy' ;\
	mv fsck $(ROOT)/usr/src/proto/i386/at386/fsck.ufs;\
	$(MAKE) -f fsck.mk clobber )
	$(STRIP) fsck.ufs
	echo usr/src/proto/i386/at386/fsck.ufs | sh ../prep NO_UNIX

fsck.s5.dy:
	( cd $(ROOT)/usr/src/cmd/fs.d/s5;\
	$(MAKE) -f s5.mk fsck ROOTLIBS='-dy' ;\
	mv fsck $(ROOT)/usr/src/proto/i386/at386/fsck.s5;\
	$(MAKE) -f s5.mk clobber )
	$(STRIP) fsck.s5
	echo usr/src/proto/i386/at386/fsck.s5 | sh ../prep NO_UNIX

$(FSBFS):
	$(STRIP) $(ROOT)/etc/fs/bfs/fsck
	echo etc/fs/bfs/fsck | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/bfs/fsck fsck.bfs
	$(STRIP) $(ROOT)/etc/fs/bfs/mount
	echo etc/fs/bfs/mount | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/bfs/mount mount.bfs
	$(STRIP) $(ROOT)/etc/fs/bfs/mkfs
	echo etc/fs/bfs/mkfs | sh ../prep NO_UNIX
	cp $(ROOT)/etc/fs/bfs/mkfs mkfs.bfs

chan.ap: $(ROOT)/etc/ap/chan.ap
	cp $(ROOT)/etc/ap/chan.ap .

at386 att compaq: $(ROOT)/etc/initprog/$@
	echo etc/initprog/$@ | sh ../prep NO_UNIX
	-$(STRIP) $(ROOT)/etc/initprog/$@
	cp $(ROOT)/etc/initprog/$@ .

newdev:
	rm -f drive_info FLOPPY/.floppydate LABEL

clean:
	/bin/rm -f rm
	rm -f $(TEXTFILES) LABEL TIMEZONE $(BOOTS)
	rm -f $(AT386PROGS) $(USRBIN) $(USRSBIN) $(FSS5) $(FSUFS) $(FSCK_SP) $(FSBFS) $(SBIN) chan.ap $(USRLIB)
	rm -f $(DFLT_BOOTS) att compaq at386 

clobber: clean newissue newdev
	cd cmd; make -f cmd.mk clobber; cd ..
	rm -f DISTLIST unix INSTALL INSTALL2 make_flops INSTALL3 $(ROOT)/tmp/INSTALL3 $(ROOT)/tmp/Plist.dev Plist.dev initial ../initial
	rm -f boot_make ask_drive $(ROOT)/var/sadm/install/contents
	rm -f .packagedate $(ROOT)/etc/inittab3 inittab3
