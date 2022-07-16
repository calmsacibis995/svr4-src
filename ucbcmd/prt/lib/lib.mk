#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#       Portions Copyright(c) 1988, Sun Microsystems, Inc.
#       All Rights Reserved

#ident	"@(#)ucbprt:lib/lib.mk	1.2.1.1"

#       Makefile for libcom.a

AR = ar

LORDER = lorder

LIBRARY = libcom.a

FILES = dofile.o		\
		date_ab.o	\
		fatal.o		\
		setsig.o	\
		xopen.o		\
		fdfopen.o	\
		xmsg.o		\
		cat.o		\
		repl.o		\
		trnslat.o	\
		clean.o		\
		dname.o		\
		sname.o		\
		imatch.o	\
		userexit.o	

all: $(LIBRARY)

$(LIBRARY): $(FILES)
	$(AR) cr $(LIBRARY) `$(LORDER) *.o | tsort`

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBRARY)
