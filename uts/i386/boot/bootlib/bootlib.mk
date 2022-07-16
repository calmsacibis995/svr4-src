#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)boot:boot/bootlib/bootlib.mk	1.1.3.1"
MORECPP = -D$(BUS) -D$(ARCH) #-DDEBUG
CFLAGS = -O -I$(INCRT) $(MORECPP)

SRCS =  s5filesys.c elf.c filesys.c blfile.c bfsfilesys.c

OBJS =  s5filesys.o elf.o filesys.o blfile.o bfsfilesys.o

.SUFFIXES: .fd .hd

.c.fd:	
	${CC} ${CFLAGS} -S -c $*.c
	sed -f ../tool/boot.sed  $*.s > $*.i
	${AS} -o $*.fd $*.i
	rm -f $*.i $*.s

.c.hd:	
	${CC} ${CFLAGS} -DWINI -S -c $*.c
	sed -f ../tool/boot.sed  $*.s > $*.i
	${AS} -o $*.hd $*.i
	rm -f $*.i $*.s

all install: tools fdbootlib hdbootlib
	
tools:	../tool/boot.sed

fdbootlib: ${OBJS:.o=.fd} 

hdbootlib: ${OBJS:.o=.hd} 

install: all

clean:
	rm -f *.o *.fd *.hd *.i

clobber: clean
