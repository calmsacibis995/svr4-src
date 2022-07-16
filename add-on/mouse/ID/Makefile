#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mouse:ID/Makefile	1.3.1.1"

ID =

IDENT = \
	IDlist \
	System.mse \
	System.bmse \
	System.smse \
	System.m320 \
	Node.mse \
	Node.bmse \
	Node.m320 \
	Rc.smse \
	Sd.smse \
	Master.mse \
	Master.bmse \
	Master.smse \
	Master.m320


all:	$(ID) $(IDENT) $(FRC)

install pkg:	all	$(FRC)
	-ls $(INSTALL) $(ID) | cpio -pduv ../pkg
	for i in $(IDENT); \
	do \
		grep -v "^#" $$i.sh | grep -v "^$$" > ../pkg/$$i ;\
	done

clean:	$(FRC)
	
clobber: clean
	rm -f $(IDENT)

FRC:
