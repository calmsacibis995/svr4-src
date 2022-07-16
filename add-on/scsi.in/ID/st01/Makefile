#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:ID/st01/Makefile	1.3"

DRIVER = st01

INSTALL=

ID = \
	Space.c

IDENT = \
	Node \
	System \
	Master


all:	$(INSTALL) $(ID) $(IDENT) $(FRC)

install :	all	$(FRC)
	[ -d ../../pkg/$(DRIVER) ] || mkdir ../../pkg/$(DRIVER)
	-ls $(INSTALL) $(ID) | cpio -pduv ../../pkg/$(DRIVER)
	for i in $(IDENT); \
	do \
		grep -v "^#" $$i.sh | grep -v "^$$" > ../../pkg/$(DRIVER)/$$i ;\
	done

clean:	$(FRC)
	
clobber: clean
	rm -f $(IDENT)

FRC:
