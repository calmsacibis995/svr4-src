#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/filecab.mk	1.5"

USR=$(ROOT)/$(HOME)
CFLAGS= -O
LDFLAGS= -s
LIBPW=$(LIBPW)

DIRS =	fileb
VHOME=$(USR)/vmsys
OHOME=$(USR)/oasys
STANDARD=$(VHOME)/standard
VDIRS= $(VHOME)/standard  $(VHOME)/standard/WASTEBASKET $(VHOME)/standard/pref
ODIRS=$(OHOME)  $(OHOME)/info $(OHOME)/info/OH $(OHOME)/info/OH/externals $(OHOME)/tmp
TERRLOG = $(OHOME)/tmp/TERRLOG
MKDIR = mkdir

SFILES= .faceprofile
WFILES=WASTEBASKET/.pref 
PFILES=pref/.environ pref/.variables pref/.colorpref
OAFILES=allobjs detect.tab
INFILES=pathalias

all:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in filecab/$$d subsystem;\
		$(MAKE) -f $$d.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBPW=$(LIBPW) HOME=$(HOME) OWNER=$(OWNER) $@;\
		cd ..;\
	done


install:	all vdirs odirs
		@set -e;\
		for d in $(DIRS);\
		do\
			cd $$d;\
			echo Making $@ in filecab/$$d subsystem;\
			$(MAKE) -f $$d.mk HOME=$(HOME) OWNER=$(OWNER) $@;\
			cd ..;\
		done

		for i in $(SFILES);\
		do\
			install -f $(STANDARD) standard/$$i;\
			chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(WFILES);\
		do\
			install -f $(STANDARD)/WASTEBASKET standard/$$i;\
			chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(PFILES);\
		do\
			install -f $(STANDARD)/pref standard/$$i;\
			chmod 664 $(STANDARD)/$$i;\
		done

		for i in $(INFILES);\
		do\
			install -f $(VHOME) $$i;\
			chmod 664 $(VHOME)/$$i;\
		done

		for i in $(OAFILES);\
		do\
			install -f $(OHOME)/info/OH/externals oasys/info/OH/externals/$$i;\
			chmod 664 $(OHOME)/info/OH/externals/$$i;\
		done


clean:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		$(MAKE) -f $$d.mk HOME=$(HOME) OWNER=$(OWNER) clean $@;\
		cd ..;\
	done

clobber: clean
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		$(MAKE) -f $$d.mk HOME=$(HOME) OWNER=$(OWNER) clobber $@;\
		cd ..;\
	done

vdirs: 	$(VDIRS)

odirs:  $(ODIRS) $(TERRLOG)

$(VDIRS):
		$(MKDIR) $@
		chmod 775 $@

$(ODIRS):
		$(MKDIR) $@
		chmod 775 $@

$(TERRLOG):
	> $@ ;\
	chmod 622 $@
