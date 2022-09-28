#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pcintf:pci.mk	1.1"
#	Copyright (c) 1989  Locus Computing Corporation
#	All Rights Reserved
#
#	@(#)pci.mk	1.4	2/26/90 18:27:36

PCI=$(ROOT)/usr/pci
PCIBIN=$(PCI)/bin
USRBIN=$(ROOT)/usr/bin
ETC=$(ROOT)/etc

all:
	cd bridge; make
	cd   util; make

install: all
	-rm -rf $(PCI)
	-rm -f $(USRBIN)/dos2unix $(USRBIN)/unix2dos $(ETC)/rc[0-3].d/[SK]95pci
	[ -d $(ETC)       ] || mkdir $(ETC)
	[ -d $(ETC)/rc0.d ] || mkdir $(ETC)/rc0.d
	[ -d $(ETC)/rc1.d ] || mkdir $(ETC)/rc1.d
	[ -d $(ETC)/rc2.d ] || mkdir $(ETC)/rc2.d
	[ -d $(ETC)/rc3.d ] || mkdir $(ETC)/rc3.d
	[ -d $(PCI)       ] || mkdir $(PCI)
	[ -d $(PCIBIN)    ] || mkdir $(PCIBIN)
	[ -d $(USRBIN)    ] || mkdir $(USRBIN)
	cp support/sys5/errlogger		$(PCI)
	cp support/sys5/pcistart		$(PCIBIN)
	cp support/sys5/pcistop			$(PCIBIN)
	cp support/sys5_4/pciptys		$(PCI)
	cp support/sys5_4/S95pci		$(ETC)/rc2.d
	ln $(ETC)/rc2.d/S95pci			$(ETC)/rc3.d
	ln $(ETC)/rc2.d/S95pci			$(ETC)/rc0.d/K95pci
	ln $(ETC)/rc2.d/S95pci			$(ETC)/rc1.d/K95pci
	cp bridge/loadpci.dir/loadpci.svr4	$(PCI)/loadpci
	cp bridge/svr4eth.dir/consvr		$(PCI)/pciconsvr.eth
	cp bridge/svr4eth.dir/dossvr		$(PCI)/pcidossvr.eth
	cp bridge/svr4eth.dir/dosout		$(PCI)/pcidosout.eth
	cp bridge/svr4eth.dir/mapsvr		$(PCI)/pcimapsvr.eth
	cp bridge/svr4232.dir/dossvr		$(PCI)/pcidossvr.232
	cp bridge/svr4232.dir/dosout		$(PCI)/pcidosout.232
	cp pkg_rlock/rlockshm			$(PCI)/sharectl
	cp util/dos2unix			$(USRBIN)
	cp util/unix2dos			$(USRBIN)

clean:
	-for i in bridge/*.dir pkg_lockset pkg_rlock ;	\
	do						\
		rm -f $$i/*.o ;				\
	done
	-rm -f pkg_lockset/liblset.a pkg_rlock/librlock.a

clobber: clean
	-rm -rf bridge/*.dir
	-rm -f util/dos2unix util/unix2dos pkg_rlock/rlockshm

