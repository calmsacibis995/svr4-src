#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:uface/Makefile	1.3"

UFACE=\
	Menu.tape \
	Text.tape \
	Text.fop \
	Text.sop

IDENT=\
	admin


all:	$(UFACE) $(IDENT) $(FRC)

install package:	all	$(FRC)
	-ls $(UFACE) | cpio -pduv ../pkg
	for i in $(IDENT); \
	do \
		grep -v "^#" $$i.sh  > ../pkg/$$i ;\
	done


clean:	$(FRC)
	rm -f $(IDENT)
	
clobber: clean

FRC:
