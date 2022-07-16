#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mbus:proto/i386/mbus/cmd/cmd.mk	1.3.3.2"

INSTL   =	$(ROOT)/opt/unix
STRIP = strip
MCS   = mcs
MAKE  = make
ARCH  = MBUS
PROTO = $(ROOT)/usr/src/proto/i386/mbus
BPATH = `eval echo "$(ROOT)/xenv:$(PATH)" | sed -e 's/::/:/g' -e 's/:.:/:/g' `
MV = /bin/mv

SBIN = sync \
	uadmin \
	autopush \
	mknod \
	init \
	disksetup 

USBIN = chroot

UBIN = expr \
	date \
	uname\
	rm\
	find \
	rmdir \
	cpio \
	mkdir \
	mv \
	cat \
	tee \
	kill 

PROC = mount.proc 

BFS = mount.bfs \
	mkfs.bfs \
	fsck.bfs

UFS = mount.ufs\
	mkfs.ufs\
	fsck.ufs\
	labelit.ufs

S5 = mount.s5\
	mkfs.s5\
	fsck.s5\
	labelit.s5

TFILES = mnttab utmp wtmp
 
PKG = $(ROOT/usr/lbin/.AD.base\
	$(ROOT)/usr/lbin/.RM.base

all:  $(INSTL) $(SBIN) $(UBIN) $(USBIN) $(BFS) $(UFS) $(S5) $(PROC) $(TFILES) DISK initial $(PKG)
	cp setup $(INSTL)
	[ -d $(ROOT)/usr/lib/tape ] || mkdir $(ROOT)/usr/lib/tape
	cp $(ROOT)/usr/src/pkg/scsi.in.src/cmd/tapecntl \
		$(ROOT)/usr/lib/tape/tapecntl

install: all
	@echo "Commands installed"

$(INSTL): 
	@[ -d $(INSTL) ] ||  mkdir -p $(INSTL) || :
	
$(TFILES):$@ 
	> $@

$(SBIN): $(@)
	-$(MV) $(ROOT)/sbin/$@ $(ROOT)/sbin/$@.orig
	ROOTLIBS=' ' PERFLIBS=' ' NOSHLIBS=' ' PATH="$(BPATH)" \
	sh $(ROOT)/usr/src/:mkcmd BUS=$(BUS) ARCH=$(ARCH) $@
	$(MV) $(ROOT)/sbin/$@ $(PROTO)/cmd/
	$(MV) $(ROOT)/sbin/$@.orig $(ROOT)/sbin/$@

$(UBIN): $@
	-$(MV) $(ROOT)/usr/bin/$@ $(ROOT)/usr/bin/$@.orig
	ROOTLIBS=' ' PERFLIBS=' ' NOSHLIBS=' '  PATH="$(BPATH)" \
	sh $(ROOT)/usr/src/:mkcmd BUS=$(BUS) ARCH=$(ARCH) $@
	$(MV) $(ROOT)/usr/bin/$@ $(PROTO)/cmd
	$(MV) $(ROOT)/usr/bin/$@.orig $(ROOT)/usr/bin/$@

$(USBIN): $@
	-$(MV) $(ROOT)/usr/sbin/$@ $(ROOT)/usr/sbin/$@.orig
	ROOTLIBS=' ' PERFLIBS=' ' NOSHLIBS=' '  PATH="$(BPATH)" \
	sh $(ROOT)/usr/src/:mkcmd BUS=$(BUS) ARCH=$(ARCH) $@
	$(MV) $(ROOT)/usr/sbin/$@ $(PROTO)/cmd
	$(MV) $(ROOT)/usr/sbin/$@.orig $(ROOT)/usr/sbin/$@


 
$(PROC):	$(@)
		@echo "$(@:.bfs=)======BFS"
		cd $(ROOT)/usr/src/cmd/fs.d/proc ;\
			$(MAKE) -f proc.mk $(@:.proc=) ROOTLIBS='-dy' ;\
			cp $(@:.proc=) $(PROTO)/cmd/$@ ;\
		$(STRIP) $(PROTO)/cmd/$@
		cd $(ROOT)/usr/src/cmd/fs.d/proc ;\
			$(MAKE) -f proc.mk $(@:.proc=) ROOTLIBS='-dy' clobber

$(BFS):	$(@)
		@echo "$(@:.bfs=)======BFS"
		cd $(ROOT)/usr/src/cmd/fs.d/bfs ;\
			$(MAKE) -f bfs.mk $(@:.bfs=) ROOTLIBS='-dy' ;\
			cp $(@:.bfs=) $(PROTO)/cmd/$@ ;\
		$(STRIP) $(PROTO)/cmd/$@
		cd $(ROOT)/usr/src/cmd/fs.d/bfs ;\
			$(MAKE) -f bfs.mk $(@:.bfs=) ROOTLIBS='-dy' clobber

$(S5):	$(@)
		@echo "$(@:.s5=)======S5"
		cd $(ROOT)/usr/src/cmd/fs.d/s5 ;\
			$(MAKE) -f s5.mk $(@:.s5=) ROOTLIBS='-dy' ;\
			cp $(@:.s5=) $(PROTO)/cmd/$@ ;\
		$(STRIP) $(PROTO)/cmd/$@
		cd $(ROOT)/usr/src/cmd/fs.d/s5 ;\
			$(MAKE) -f s5.mk $(@:.s5=) ROOTLIBS='-dy' clobber

$(UFS):	$(@)
		@echo "$(@:.ufs=)======UFS"
		cd $(ROOT)/usr/src/cmd/fs.d/ufs/$(@:.ufs=) ;\
			$(MAKE) -f $(@:.ufs=).mk $(@:.ufs=) ROOTLIBS='-dy' ;\
			cp $(@:.ufs=) $(PROTO)/cmd/$@ ;\
		$(STRIP) $(PROTO)/cmd/$@
		cd $(ROOT)/usr/src/cmd/fs.d/ufs/$(@:.ufs=) ;\
			$(MAKE) -f $(@:.ufs=).mk $(@:.ufs=) ROOTLIBS='-dy' clobber 

ufs:
		@echo "======UFS"
		cd $(ROOT)/usr/src/cmd/fs.d/ufs ;\
		for i in mount mkfs fsck labelit;\
		do\
			cd $(ROOT)/usr/src/cmd/fs.d/ufs/$$i ;\
			$(MAKE) -f $$i.mk $$i ROOTLIBS='-dy' ;\
			cp $$i $(PROTO)/cmd/$$i.ufs ;\
			$(STRIP) $(PROTO)/cmd/$$i.ufs ;\
		done

DISK:
	cp disk.dfl disk.tmp
	sed '/^#/d' disk.tmp >disk.dfl

initial:
	cd ../../PACKAGES/peruser; make -f peruser.mk ../../initial; 
	cp ../../initial initial
	echo usr/src/proto/i386/mbus/cmd//initial | sh ../../prep NO_UNIX
	
$(PKG): RM.base AD.base
	cp AD.base $(ROOT)/usr/lbin/.AD.base
	cp RM.base $(ROOT)/usr/lbin/.RM.base

clean:
	rm -f  $(SBIN) $(UBIN) $(USBIN) $(BFS) $(UFS) $(S5) *.b

clobber: clean
	rm -f $(TFILES) install
