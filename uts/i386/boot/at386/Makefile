#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)boot:boot/at386/Makefile	1.1.2.1"

CPPFLAGS = -I$(INCRT) $(MORECPP)
CFLAGS = -O $(CPPFLAGS)
INSDIR = $(ROOT)/etc
SYMVALS = bsymvals
STRIP = strip

SRCS = GDT.s bstart.s util.s memcpy.s pit.s prot.s start.s touchpage.s \
	boot.c default.c disk.c gets.c load.c memsize.c \
	physaddr.c printf.c string.c 

# order is important, here; start.o must come first

OBJS = start.o prot.o GDT.o util.o pit.o boot.o printf.o gets.o  \
	load.o disk.o string.o default.o memcpy.o touchpage.o \
	physaddr.o memsize.o bstart.o 

LIBOBJS = ../bootlib/blfile.o ../bootlib/filesys.o ../bootlib/s5filesys.o \
	  ../bootlib/elf.o  ../bootlib/bfsfilesys.o

# We need all these different targets to fool make into making two different
# executables, such that the .o files are compiled with different flags.

.SUFFIXES: .fd .hd

.s.fd:
	cat ${SYMVALS}.s $*.s > $*2.s
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		${CPP} -P ${CPPFLAGS} $*2.s $*2.i ; \
	else \
		${CC} -P ${CPPFLAGS} $*2.s ;\
	fi
	${AS} -o $*.fd $*2.i
	-/bin/rm $*2.i $*2.s

.c.fd:	
	${CC} ${CFLAGS} -S -c $*.c
	sed -f ../tool/boot.sed  $*.s > $*.i
	${AS} -o $*.fd $*.i
	-/bin/rm $*.i $*.s

.s.hd:
	cat ${SYMVALS}.s $*.s > $*2.s
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		${CPP} -P ${CPPFLAGS} -DWINI $*2.s $*2.i ; \
	else \
		${CC} -P ${CPPFLAGS} -DWINI $*2.s ;\
	fi
	${AS} -o $*.hd $*2.i
	-/bin/rm $*2.i $*2.s

.c.hd:	
	${CC} ${CFLAGS} -DWINI -S -c $*.c
	sed -f ../tool/boot.sed  $*.s > $*.i
	${AS} -o $*.hd $*.i
	-/bin/rm $*.i $*.s

all: tools ${SYMVALS}.s initprogs fdboot hdboot
	
tools:	../tool/boot.sed
	cd tool; make -f tools.mk

initprogs: 
	cd initprog; make -f initprog.mk

fdboot: ${OBJS:.o=.fd} 
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${LD} -Mmapfile -dn -o fdboot ${OBJS:.o=.fd} ${LIBOBJS:.o=.fd}; \
	else \
		${LD} -N -o fdboot ${OBJS:.o=.fd} ${LIBOBJS:.o=.fd}; \
	fi 
	${STRIP} fdboot
	tool/rmhdr fdboot fdboot.obj
	mv fdboot.obj fdboot
#	tool/doitfd fdboot.obj

hdboot: ${OBJS:.o=.hd} 
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${LD} -Mmapfile -dn -o hdboot ${OBJS:.o=.hd} ${LIBOBJS:.o=.hd}; \
	else \
		${LD} -N -o hdboot ${OBJS:.o=.hd} ${LIBOBJS:.o=.hd}; \
	fi 
	${STRIP} hdboot
#	tool/doithd hdboot

install: all
	cpset fdboot $(INSDIR)/.fboot 0644 bin bin
	cpset hdboot $(INSDIR)/.wboot 0644 bin bin
	cd initprog; make -f initprog.mk install
	cd default; make -f default.mk install

${SYMVALS}.s:	Gensymvals \
	${INCRT}/sys/types.h \
	${INCRT}/sys/bootinfo.h \
	../sys/boot.h
	-sh ./Gensymvals "CC=${CC}" "CFLAGS=${CFLAGS}"

clean:
	-/bin/rm *.o *.fd *.hd *.i ${SYMVALS}.?
	cd initprog; make -f initprog.mk clean
	cd default; make -f default.mk clean
	cd tool; make -f tools.mk clean

clobber: clean
	-/bin/rm hdboot fdboot fdboot.obj
	cd initprog; make -f initprog.mk clobber
	cd default; make -f default.mk clobber
	cd tool; make -f tools.mk clobber
