#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:xl.mk	1.3"

DIRECTORIES=\
	io \
	cmd	\
	sys \
	ID \
	uface\
	pkg

all install clobber clean:	$(FRC)
	for i in $(DIRECTORIES); \
	do \
		cd $$i; echo "$$i\t\c"; make $@ ; cd .. ; \
	done
	@echo "\t\t$@ - done"

package:
	cd pkg; make package; cd ..
FRC:
