#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi:ID/Makefile	1.3"

SUBDIR = \
	scsi \
	sd01

SPACE = \
	scsi/space.c \
	sd01/space.c

IDENTid = \
	scsi/Master \
	scsi/System \
	scsi/System2 \
	scsi/System3 \
	sd01/Master \
	sd01/Master \
	sd01/System 

all:	$(SUBDIR) $(SPACE) $(IDENTid) $(FRC)

install package:	all	$(FRC)
	[ -d ../pkg ] || mkdir ../pkg
	chmod 0777 ../pkg; \
	for i in $(SUBDIR); \
	do \
		[ -d ../pkg/$$i ] || mkdir ../pkg/$$i; \
		chmod 0777 ../pkg/$$i; \
		cp $$i/space.c ../pkg/$$i; \
	done

	for i in $(IDENTid); \
	do \
		echo $$i; \
		grep -v "^#" $$i | grep -v "^$$" > ../pkg/$$i ;\
	done
clean:	$(FRC)
	
clobber: clean
	rm -f $(IDENTid)

FRC:
