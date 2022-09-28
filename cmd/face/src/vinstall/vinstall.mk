#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/vinstall/vinstall.mk	1.4"
#
# vinstall.mk -- the makefile for the install subsystem of FACE
#

USR =	$(ROOT)/$(HOME)

LCMDS = vsetup vmodify vdelete addmenu delmenu

VBIN = $(USR)/vmsys/bin

BIN = $(USR)/vmsys $(USR)/vmsys/bin $(USR)/oasys $(USR)/oasys/bin

MKDIR = mkdir

TMP = $(USR)/oasys/tmp

TERRLOG = $(TMP)/TERRLOG

all:		compile

compile:  	$(LCMDS)

install: 	compile  $(BIN) $(TMP) $(TERRLOG)
		@set +e;\
		for i in $(LCMDS);\
		do\
			install -f $(VBIN) $$i;\
		done

clean:
	@set +e;\
	for i in $(LCMDS);\
	do\
		/bin/rm -f $$i;\
	done

clobber:	clean

$(BIN):
	$(MKDIR) $@ ;\
	chmod 755 $@

$(TMP):
	$(MKDIR) $@ ;\
	chmod 777 $@

$(TERRLOG):
	> $@ ;\
	chmod 622 $@

.sh :
	rm -f $@
	/bin/cp $< $@
	chmod 755 $@
