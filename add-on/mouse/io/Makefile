#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mouse:io/Makefile	1.3"
#

MORECPP=-DAT386 -D_KERNEL -DSYSV

all:	Driver.o.mse Driver.o.bmse Driver.o.smse Driver.o.m320

install package:	all 
	-ls Space.c.bmse Driver.o.mse Driver.o.bmse Driver.o.smse Driver.o.m320 mse.h | cpio -pduv ../pkg

clean: $(FRC)
	rm -f mouse.o smse.o bmse.o m320.o mse_subr.o

clobber: clean
	rm -f Driver.o.mse Driver.o.bmse Driver.o.smse Driver.o.m320

Driver.o.mse:	mouse.o mse_subr.o
	$(LD) -r -o $@ mouse.o mse_subr.o

Driver.o.m320:	m320.o 
	$(LD) -r -o $@ m320.o

Driver.o.bmse:	bmse.o 
	$(LD) -r -o $@ bmse.o

Driver.o.smse:	smse.o 
	$(LD) -r -o $@ smse.o

mouse.o:	mouse.c mse.h $(FRC)
	$(CC) -c -O $(MORECPP) mouse.c

smse.o:	smse.c mse.h $(FRC)
	$(CC) -c -O $(MORECPP) smse.c

bmse.o:	bmse.c mse.h $(FRC)
	$(CC) -c -O $(MORECPP) bmse.c

m320.o:	m320.c mse.h $(FRC)
	$(CC) -c -O $(MORECPP) m320.c

mse_subr.o:	mse_subr.c mse.h $(FRC)
	$(CC) -c -O $(MORECPP) mse_subr.c
	cp mse.h $(ROOT)/usr/include/sys

FRC:
