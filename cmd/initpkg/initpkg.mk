#ident	"%W%	%E%	%Q%"

#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)initpkg:./initpkg.mk	1.19.11.1"

ROOT =
INSDIR = $(ROOT)/sbin
TOUCH=$(CH)/bin/touch
SYMLINK = :
INS = install
MAKE=make
ARCH=AT386
BUS=AT386

SCRIPTS= bcheckrc brc inittab vfstab rstab mountall\
	 rc0 rc1 rc2 rc3 rc6\
	 rmount rmountall rumountall shutdown umountall\
	 dumpsave
DIRECTORIES= init.d rc0.d rc1.d rc2.d rc3.d
OTHER= dumpcheck
DIRS = \
	$(ROOT)/etc \
	$(ROOT)/etc/init.d \
	$(ROOT)/etc/dfs \
	$(ROOT)/etc/rc0.d \
	$(ROOT)/etc/rc1.d \
	$(ROOT)/etc/rc2.d \
	$(ROOT)/etc/rc3.d

all:	scripts directories other $(DIRS)

scripts: $(SCRIPTS)

directories: $(DIRECTORIES)

other: $(OTHER)

clean:

clobber: clean
	rm -f $(SCRIPTS) $(OTHER) dfstab

install: $(DIRS) all

$(DIRS):
	mkdir $@

bcheckrc::
	-rm -f $(ROOT)/etc/$@
	-rm -f $(ROOT)/usr/sbin/$@
	sh $@.sh $@
	$(INS) -f $(INSDIR) -m 0744 -u root -g sys $@
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g sys $@
	$(TOUCH) 0101000070 $(INSDIR)/$@
	-$(SYMLINK) /sbin/$@ $(ROOT)/etc/$@
	
brc::
	-rm -f $(ROOT)/etc/$@
	-rm -f $(ROOT)/usr/sbin/$@
	sh $@.sh $(ROOT)
	$(INS) -f $(INSDIR) -m 0744 -u root -g sys $@
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g sys $@

dumpsave::
	sh $@.sh $(ROOT)
	$(INS) -f $(INSDIR) -m 0744 -u root -g sys $@
	$(TOUCH) 0101000070 $(INSDIR)/$@

dumpcheck:	dumpcheck.c
	$(CC) -D$(ARCH) -D$(BUS) -o dumpcheck dumpcheck.c -O -s -lcmd $(ROOTLIBS)
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin dumpcheck
	$(INS) -f $(ROOT)/etc/default -m 0644 -u bin -g bin dump.dfl
	-mv $(ROOT)/etc/default/dump.dfl $(ROOT)/etc/default/dump

vfstab::
	sh $@.sh  $(ROOT)
	$(INS) -f $(ROOT)/etc -m 0744 -u root -g sys $@
	$(TOUCH) 0101000070 $(ROOT)/etc/$@

rstab::
	sh rstab.sh  $(ROOT)
	$(INS) -f $(ROOT)/etc/dfs -m 0644 -u root -g sys dfstab
	$(TOUCH) 0101000070 $(ROOT)/etc/dfs/dfstab

inittab::
	sh inittab.sh $(ROOT)
	$(INS) -f $(ROOT)/etc  -m 0664 -u root -g sys inittab
	$(TOUCH) 0101000070 $(ROOT)/etc/inittab

mountall::
	-rm -f $(ROOT)/etc/mountall
	-rm -f $(ROOT)/usr/sbin/mountall
	cp mountall.sh  ./mountall
	$(INS) -f $(INSDIR) -m 0555 -u root -g sys mountall
	$(TOUCH) 0101000070 $(INSDIR)/mountall
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u root -g sys mountall
	-$(SYMLINK) /sbin/mountall $(ROOT)/etc/mountall

rc0 rc1 rc2 rc3 rc6::
	-rm -f $(ROOT)/etc/$@
	-rm -f $(ROOT)/usr/sbin/$@
	sh $@.sh $(ROOT)
	$(INS) -f $(INSDIR) -m 0744 -u root -g sys $@
	$(INS) -f $(ROOT)/usr/sbin -m 0744 -u root -g sys $@
	$(TOUCH) 0101000070 $(INSDIR)/$@
	-$(SYMLINK) /sbin/$@ $(ROOT)/etc/$@


rmount::
	-rm -f $(ROOT)/etc/rmount
	sh rmount.sh $(ROOT)
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u root -g sys rmount
	$(TOUCH) 0101000070 $(ROOT)/usr/sbin/rmount
	-$(SYMLINK) /usr/sbin/rmount $(ROOT)/etc/rmount

rmountall rumountall::
	-rm -f $(ROOT)/etc/$@
	sh $@.sh $(ROOT)
	$(INS) -f $(ROOT)/usr/sbin -m 544 -u root -g sys $@
	$(TOUCH) 0101000070 $(ROOT)/usr/sbin/$@
	-$(SYMLINK) /usr/sbin/$@ $(ROOT)/etc/$@

shutdown::
	-rm -f $(ROOT)/etc/shutdown
	-rm -f $(ROOT)/usr/sbin/shutdown
	cp shutdown.sh shutdown
	$(INS) -f $(INSDIR) -m 0755 -u root -g sys shutdown
	$(INS) -f $(ROOT)/usr/sbin -m 0755 -u root -g sys shutdown
	$(TOUCH) 0101000070 $(INSDIR)/shutdown
	-$(SYMLINK) /sbin/shutdown $(ROOT)/etc/shutdown

umountall::
	-rm -f $(ROOT)/etc/umountall
	-rm -f $(ROOT)/usr/sbin/umountall
	cp umountall.sh  umountall
	$(INS) -f $(INSDIR) -m 0555 -u root -g sys umountall
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u root -g sys umountall
	$(TOUCH) 0101000070 $(INSDIR)/umountall
	-$(SYMLINK) /sbin/umountall $(ROOT)/etc/umountall

init.d rc0.d rc1.d rc2.d rc3.d::
	cd ./$@; \
	sh :mk.$@.sh ROOT=$(ROOT) CH=$(CH)
