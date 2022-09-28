#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/src.mk	1.4"

CFLAGS= -O
LDFLAGS= -s
LIBPW=$(LIBPW)

DIRS =	proc xx filecab vinstall oam

all:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		$(MAKE) -f $$d.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" VMSYS="$(VMSYS)" VMBIN="$(VMBIN)" LIBPW=$(LIBPW) HOME=$(HOME) OWNER=$(OWNER) $@;\
		cd ..;\
	done

install: all
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo Making $@ in $$d subsystem;\
		$(MAKE) -f $$d.mk CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" VMSYS="$(VMSYS)" VMBIN="$(VMBIN)" LIBPW=$(LIBPW) HOME=$(HOME) OWNER=$(OWNER) $@;\
		cd ..;\
	done


clean:
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		$(MAKE) -f $$d.mk VMSYS="$(VMSYS)" VMBIN="$(VMBIN)" HOME=$(HOME) OWNER=$(OWNER) clean $@;\
		cd ..;\
	done

clobber: clean
	@set -e;\
	for d in $(DIRS);\
	do\
		cd $$d;\
		echo "\nMaking $@ in $$d subsystem\n";\
		$(MAKE) -f $$d.mk VMSYS="$(VMSYS)" VMBIN="$(VMBIN)" HOME=$(HOME) OWNER=$(OWNER) clobber $@;\
		cd ..;\
	done
