#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:uts/i386/boot/msa/msa.mk	1.3.1.1"

CPP		=/lib/cpp
CCSTYPE	= ELF
INCRT	= ../..
INCS	= $(ROOT)/usr/include
ROOT	=
DASHO   = -O
BUS     = MB2
LOCALCPP= -DMB2SA #-DDEBUG
MORECPP	= -D$(BUS) -D$(ARCH)
CFLAGS	= $(DASHO) $(MORECPP) $(LOCALCPP) -I$(INCRT) -I$(INCS)
LDFLAGS	=

# these object files should remain in this order - omf286.o must be
# in the first 64K of memory
 
OBJ		= s2main.o omf286.o bpsmgr.o cfgfmgr.o pload.o ics.o pboot.o msg.o  \
		  string.o omf386.o util.o printf.o ../bootlib/elf.hd
DOBJ	= ../bootlib/blfile.hd ../bootlib/filesys.hd ../bootlib/bfsfilesys.hd \
		  ../bootlib/s5filesys.hd disk.o
TOBJ	= tapemgr.o
BOBJ	= bsmgr.o bsfread.o
EXECS	= dsboot tsboot bsboot

INSDIR	= $(ROOT)/etc

all: dsboot tsboot bsboot 

dsboot: $(OBJ) $(DOBJ)
	if [ x${CCSTYPE} = xELF ]; then \
		$(LD) $(LDFLAGS) -dn -Mmapfile -e s2main -o $@ $(OBJ) $(DOBJ);\
	else \
		$(LD) $(LDFLAGS) -e s2main -o $@ $(OBJ) $(DOBJ) ifile ; \
	fi 
	sh checkbss $@

tsboot: $(OBJ) $(TOBJ)
	if [ x${CCSTYPE} = xELF ]; then \
		$(LD) $(LDFLAGS) -dn -Mmapfile -e s2main -o $@ $(OBJ) $(TOBJ);\
	else \
		$(LD) $(LDFLAGS) -e s2main -o $@ $(OBJ) $(TOBJ) ifile ; \
	fi 
	sh checkbss $@

bsboot: $(OBJ) $(BOBJ) 
	if [ x${CCSTYPE} = xELF ]; then \
		$(LD) $(LDFLAGS) -dn -Mmapfile -e s2main -o $@.tmp $(OBJ) $(BOBJ);\
	else \
		$(LD) $(LDFLAGS) -e s2main -o $@.tmp $(OBJ) $(BOBJ) ifile ; \
	fi 
	sh checkbss $@.tmp
	[ $(BUS) = "MB1" ] || $(ROOT)/sbin/sgib -B -M $@.tmp $@
	rm -f $@.tmp

.c.o:
	$(CC) -S $(CFLAGS) $*.c
	sed -f ../tool/boot.sed  $*.s >tmp.s
	$(AS) -o $@ tmp.s
	rm $*.s tmp.s

.s.o:
	$(CPP) $(MORECPP) -I$(INCRT) -I$(INCS) -P $? $*.i
	$(AS) -o $@ $*.i
	rm $*.i

clean:
	rm -f $(OBJ) $(BOBJ) $(DOBJ) $(TOBJ)

clobber: clean  
	rm -f $(EXECS)

install: all
	install -f  $(INSDIR) -m 0644 -u root -g sys dsboot 
	install -f  $(INSDIR) -m 0644 -u root -g sys tsboot 
	[ $(BUS) = "MB1" ] || install -f $(INSDIR) -m 0644 -u root -g sys bsboot 
