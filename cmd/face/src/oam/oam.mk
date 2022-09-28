#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/oam/oam.mk	1.5"

GRP=sys
PKGINST=face
OAMBASE=$(ROOT)/usr/sadm/sysadm
PKGSAV=$(ROOT)/var/sadm/pkg/$(PKGINST)/save
INST=$(OAMBASE)/add-ons/$(PKGINST)
FACE=$(INST)/applmgmt/FACE

install:
	-mkdir -p $(PKGSAV)/intf_install
	-mkdir -p $(FACE)
	-mkdir $(FACE)/mail_services
	-mkdir $(FACE)/mail_services/add
	-mkdir $(FACE)/mail_services/remove
	-mkdir $(FACE)/dos_programs
	-mkdir $(FACE)/dos_programs/add
	-mkdir $(FACE)/dos_programs/list
	-mkdir $(FACE)/dos_programs/modify
	-mkdir $(FACE)/dos_programs/remove
	-mkdir $(FACE)/programs
	-mkdir $(FACE)/programs/add
	-mkdir $(FACE)/programs/list
	-mkdir $(FACE)/programs/modify
	-mkdir $(FACE)/programs/remove
	-mkdir $(FACE)/spell_checker
	-mkdir $(FACE)/spell_checker/add
	-mkdir $(FACE)/spell_checker/remove
	-mkdir $(FACE)/users
	-mkdir $(FACE)/users/add
	-mkdir $(FACE)/users/list
	-mkdir $(FACE)/users/modify
	-mkdir $(FACE)/users/remove
	cp face.mi $(PKGSAV)/intf_install/face.mi
	cp Menu.FACE $(FACE)
	cp Menu.programs $(FACE)/programs
	cp Menu.dos $(FACE)/dos_programs
	cp Menu.users $(FACE)/users
	cp Menu.spell $(FACE)/spell_checker
	cp Menu.mail $(FACE)/mail_services
	cp dos/Form.add	$(FACE)/dos_programs/add
	cp dos/Menu.list	$(FACE)/dos_programs/list
	cp dos/Menu.mod	$(FACE)/dos_programs/modify
	cp dos/Form.mod	$(FACE)/dos_programs/modify
	cp dos/Menu.del	$(FACE)/dos_programs/remove
	cp dos/Text.conf	$(FACE)/dos_programs/remove
	cp programs/Text.amail	$(FACE)/mail_services/add
	cp programs/Text.dmail	$(FACE)/mail_services/remove
	cp programs/Form.add	$(FACE)/programs/add
	cp programs/Menu.list	$(FACE)/programs/list
	cp programs/Menu.mod	$(FACE)/programs/modify
	cp programs/Form.mod	$(FACE)/programs/modify
	cp programs/Menu.del	$(FACE)/programs/remove
	cp programs/Text.conf	$(FACE)/programs/remove
	cp programs/Text.aspell	$(FACE)/spell_checker/add
	cp programs/Text.dspell	$(FACE)/spell_checker/remove
	cp users/Form.define	$(FACE)/users/add
	cp users/Text.define	$(FACE)/users/add
	cp users/Menu.ulist	$(FACE)/users/list
	cp users/Form.mod	$(FACE)/users/modify
	cp users/Text.mod	$(FACE)/users/modify
	cp users/Form.undefine	$(FACE)/users/remove
	cp users/Text.undefine	$(FACE)/users/remove
	cp FACE.help $(FACE)/Help
	cp dos/dos.help	$(FACE)/dos_programs/Help
	cp dos/add.help	$(FACE)/dos_programs/add/Help
	cp dos/list.help	$(FACE)/dos_programs/list/Help
	cp dos/modify.help	$(FACE)/dos_programs/modify/Help
	cp dos/remove.help	$(FACE)/dos_programs/remove/Help
	cp programs/mail_svc.help $(FACE)/mail_services/Help
	cp programs/tamail.help	$(FACE)/mail_services/add/Help
	cp programs/tdmail.help	$(FACE)/mail_services/remove/Help
	cp programs/programs.help $(FACE)/programs/Help
	cp programs/fadd.help	$(FACE)/programs/add/Help
	cp programs/mlist.help	$(FACE)/programs/list/Help
	cp programs/fmod.help	$(FACE)/programs/modify/Help
	cp programs/fdel.help	$(FACE)/programs/remove/Help
	cp programs/spell_ck.help	$(FACE)/spell_checker/Help
	cp programs/taspell.help	$(FACE)/spell_checker/add/Help
	cp programs/tdspell.help	$(FACE)/spell_checker/remove/Help
	cp users/users.help $(FACE)/users/Help
	cp users/fdefine.help	$(FACE)/users/add/Help
	cp users/mulist.help	$(FACE)/users/list/Help
	cp users/fmod.help	$(FACE)/users/modify/Help
	cp users/fundefine.help	$(FACE)/users/remove/Help
	$(CH)-chgrp $(GRP) $(PKGSAV)/intf_install/face.mi
	-chmod 644 $(PKGSAV)/intf_install/face.mi
	$(CH)-chown $(OWNER) $(PKGSAV)/intf_install/face.mi
	$(CH)-chgrp $(GRP) $(FACE)/*/*/*
	-chmod 755 $(FACE)/*/*/*
	$(CH)-chown $(OWNER) $(FACE)/*/*/*
	$(CH)-chgrp $(GRP) $(FACE)/*/*
	-chmod 755 $(FACE)/*/*
	$(CH)-chown $(OWNER) $(FACE)/*/*
	$(CH)-chgrp $(GRP) $(FACE)/*
	-chmod 755 $(FACE)/*
	$(CH)-chown $(OWNER) $(FACE)/*
	$(CH)-chgrp $(GRP) $(FACE)
	-chmod 755 $(FACE)
	$(CH)-chown $(OWNER) $(FACE)
	
clobber:

all:

clean:
