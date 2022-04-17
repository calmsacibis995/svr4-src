#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bnu.admin:bnu.admin.mk	2.26.5.4"

#	This makefile makes appropriate directories
#	and copies the simple admin shells into
#	the uucpmgmt directory
#
OWNER=root
GRP=sys
PKG=$(ROOT)/var/sadm/pkg
PKGINST=bnu
PKGBNU=$(PKG)/$(PKGINST)
PKGSAV=$(PKGBNU)/sav
PKGMI=$(PKGSAV)/intf_install
OAMBASE=$(ROOT)/usr/sadm/sysadm
ADDONS=$(OAMBASE)/add-ons
MENU=$(OAMBASE)/menu
BNU=$(ADDONS)/$(PKGINST)
NETSERV=$(MENU)/netservices
BASIC_NET=$(MENU)/netservices/bnu
BIN=$(OAMBASE)/bin
DEVICE=$(BASIC_NET)/devices
DEVADD=$(DEVICE)/add
DEVLIST=$(DEVICE)/list
DEVRM=$(DEVICE)/remove
POLLING=$(BASIC_NET)/polling
POLLADD=$(POLLING)/add
POLLIST=$(POLLING)/list
POLLRM=$(POLLING)/remove
SYS=$(BASIC_NET)/systems
SYSADD=$(SYS)/add
SYSLIST=$(SYS)/list
SYSRM=$(SYS)/remove
SET=$(BASIC_NET)/setup
SETADD=$(SET)/add
INS=install
DIRS=\
	$(MENU) \
	$(BASIC_NET) \
	$(DEVICE) \
	$(DEVADD) \
	$(DEVLIST) \
	$(DEVRM) \
	$(NETSERV) \
	$(POLLING) \
	$(POLLADD) \
	$(POLLIST) \
	$(POLLRM) \
	$(SET) \
	$(SETADD) \
	$(SYS) \
	$(SYSADD) \
	$(SYSLIST) \
	$(SYSRM) \
	$(PKG) \
	$(PKGBNU) \
	$(PKGSAV) \
	$(PKGMI) \
	$(BIN)

install: $(DIRS)
	-@rm -fr tmp
	-mkdir tmp
	$(INS) -f $(NETSERV) -m 0644 -u $(OWNER) -g $(GRP) netserve.menu
	$(INS) -f $(BASIC_NET) -m 0644 -u $(OWNER) -g $(GRP) bnu.menu
	$(INS) -f $(DEVICE) -m 0644 -u $(OWNER) -g $(GRP) devices.menu
	$(INS) -f $(POLLING) -m 0644 -u $(OWNER) -g $(GRP) polling.menu
	$(INS) -f $(SYS) -m 0644 -u $(OWNER) -g $(GRP) systems.menu
	$(INS) -f $(SET) -m 0644 -u $(OWNER) -g $(GRP) setup.menu
	rm -f tmp/Help ; ln Helpbnu tmp/Help ; \
	$(INS) -f $(BASIC_NET) -m 0644 -u $(OWNER) -g $(GRP) tmp/Help
	rm -f tmp/Help ; ln Helpdev tmp/Help ; \
	$(INS) -f $(DEVICE) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	rm -f tmp/Help ; ln Helpdev_a tmp/Help ; \
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.devtype
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.modtype
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev1
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev2
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev3
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev4
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acudev5
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.adddev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.dirdev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.othdev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.streams
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.tlisdev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_adev
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.ports
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Text.aok
	$(INS) -f $(DEVADD) -m 0755 -u $(OWNER) -g $(GRP) Text.anok
	#
	rm -f tmp/Help ; ln Helpdev_l tmp/Help ;\
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lsdev
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lsdev_1
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lsdev_2
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lsdev_all
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.ports
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Form.listdev
	$(INS) -f $(DEVLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.devname
	#
	rm -f tmp/Help ; ln Helpdev_r tmp/Help ;\
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmdev
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmdev_1
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmdev_2
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmdev_all
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Form.rmdev
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_rdev
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.ports
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmnok
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmok
	$(INS) -f $(DEVRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.devname
	#
	rm -f tmp/Help ; ln Helpol tmp/Help ;\
	$(INS) -f $(POLLING) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	rm -f tmp/Help ; ln Helpol_a tmp/Help ;\
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) Form.addpoll
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_apoll
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.sysname
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) Text.aok
	$(INS) -f $(POLLADD) -m 0755 -u $(OWNER) -g $(GRP) Text.anok
	rm -f tmp/Help ; ln Helpol_l tmp/Help ;\
	$(INS) -f $(POLLIST) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(POLLIST) -m 0755 -u $(OWNER) -g $(GRP) Form.listpoll
	$(INS) -f $(POLLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.psysname
	$(INS) -f $(POLLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lspoll
	$(INS) -f $(POLLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lspoll_a
	rm -f tmp/Help ; ln Helpol_r tmp/Help ;\
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmpoll
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmpoll_a
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_rpoll
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmnok
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmok
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Form.rmpoll
	$(INS) -f $(POLLRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.psysname
	#
	rm -f tmp/Help ; ln Helsys tmp/Help ;\
	$(INS) -f $(SYS) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	rm -f tmp/Help ; ln Helsys_a tmp/Help ;\
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.acusys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.addsys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.dirsys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.othsys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.tlissys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_asys
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.devname
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Text.aok
	$(INS) -f $(SYSADD) -m 0755 -u $(OWNER) -g $(GRP) Text.anok
	rm -f tmp/Help ; ln Helsys_l tmp/Help ;\
	$(INS) -f $(SYSLIST) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(SYSLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lssys
	$(INS) -f $(SYSLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.lssys_all
	$(INS) -f $(SYSLIST) -m 0755 -u $(OWNER) -g $(GRP) Form.listsys
	$(INS) -f $(SYSLIST) -m 0755 -u $(OWNER) -g $(GRP) Menu.sysname
	rm -f tmp/Help ; ln Helsys_r tmp/Help ;\
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmsys
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.rmsys_all
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Form.cf_rsys
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Form.rmsys
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmnok
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Text.rmok
	$(INS) -f $(SYSRM) -m 0755 -u $(OWNER) -g $(GRP) Menu.sysname
	rm -f tmp/Help ; ln Helpset tmp/Help ;\
	$(INS) -f $(SET) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	rm -f tmp/Help ; ln Helpset_a tmp/Help ;\
	$(INS) -f $(SETADD) -m 0755 -u $(OWNER) -g $(GRP) tmp/Help
	$(INS) -f $(SETADD) -m 0755 -u $(OWNER) -g $(GRP) Menu.setup
	#
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) bnuset
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) delentry
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validhour
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validls
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validname
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validnetaddr
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validphone
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validport
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) validsys
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) uniqport
	$(INS) -f $(BIN) -m 0755 -u $(OWNER) -g $(GRP) bnunotab

$(DIRS):
	-for d in $(DIRS) ; \
	do \
		[ -d "$$d" ] || mkdir -p $$d ;\
		$(CH)chmod 0755 $$d ;\
		$(CH)chgrp $(GRP) $$d ;\
		$(CH)chown $(OWNER) $$d ;\
	done

clobber:
	-rm -rf tmp
