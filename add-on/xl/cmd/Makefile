#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:cmd/Makefile	1.3"

TAPEPROGS = ftapecntl

all:	$(TAPEPROGS) $(FRC)

install package:	all $(FRC)
	-ls $(TAPEPROGS) | cpio -pduv ../pkg

clean:	$(FRC)
	rm -f *.o

clobber: clean
	rm -f $(TAPEPROGS)

ftapecntl:	ftapecntl.o 
		$(CC) -s -O -o ftapecntl ftapecntl.o

ftapecntl.o:	ftapecntl.c ../sys/ftape.h $(FRC)
		$(CC) -O -c ftapecntl.c

FRC:
