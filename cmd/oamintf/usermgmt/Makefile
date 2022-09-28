#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:usermgmt/Makefile	1.4.2.2"

MAINS=\
	lsgrp getmaxuid getdfl moddate
O_SHFILES=\
	grpchk redeflt pswdchk pswddef \
	addgrp chk_logins chk_sgrp getval getsupgrp giddflt uiddflt \
	uniq_gid uniq_uid uniq_gnam uniq_lnam chk_pgrp modgrp
O_DFILES=\
	user.menu \
	Form.defaults Text.dfltnok Text.dfltok \
	Form.password Menu.passwd Text.pswdok Text.pswdnok \
	Form.list Text.lsgrp Text.lsusr \
	Form.add Form.addgrp Form.addusr Form.addpswd Text.agrpnok Text.agrpok \
	Text.badprim Text.badsup Menu.mlogins Menu.pgrp Menu.sgrp \
	Form.modgrp1 Form.modgrp2 Form.modify Form.modusr1 Form.modusr2 \
	Text.mgrpnok Text.mgrpok Text.musrnok Text.musrok \
	Text.rgrpnok Menu.logins Menu.mlogins Menu.pgrp Menu.sgrp \
	Form.remove Form.rmgrp Form.rmusr1 Form.rmusr2 Text.rmgrp \
	Text.rgrpok Text.rusrnok Text.rusrok Menu.logins Menu.pgrp \
	Text.ausrnok Text.apswdok Text.apswdnok

HELPFILES=\
	Help add.Help defaults.Help list.Help modify.Help password.Help \
	remove.Help

HELPDEST=\
	usermgmt/Help add/Help defaults/Help list/Help modify/Help \
	password/Help remove/Help

MODDATE_SRC=moddate.c
MODDATE_OBJ=moddate.o
MODDATE=moddate
LSGRP_SRC=lsgrp.c
LSGRP_OBJ=lsgrp.o
LSGRP=lsgrp
GETMAX_SRC=getmaxuid.c
GETMAX_OBJ=getmaxuid.o
GETMAX=getmaxuid
GETDFL_SRC=getdfl.c
GETDFL_OBJ=getdfl.o
GETDFL=getdfl

all: $(O_SHFILES) $(MAINS)

$(MODDATE): $(MODDATE_OBJ)
	$(CC) -s -o $(MODDATE) $(MODDATE_OBJ)

$(LSGRP): $(LSGRP_OBJ)
	$(CC) -s -o $(LSGRP) $(LSGRP_OBJ)

$(GETMAX): $(GETMAX_OBJ)
	$(CC) -s -o $(GETMAX) $(GETMAX_OBJ)

$(GETDFL): $(GETDFL_OBJ)
	$(CC) -s -o $(GETDFL) $(GETDFL_OBJ) -lcmd
clean:
	rm -f *.o

clobber: clean
	rm -f $(O_SHFILES) $(MAINS)

size strip: all
	:

install: all xinstall

PROTO=Prototype
OBJECTS=$(HELPFILES) $(MAINS) $(O_SHFILES) $(O_DFILES)

OBJDEST=$(HELPDEST)
include ../Install
