#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Reserved

#	INTEL CORPORATION PROPRIETARY INFORMATION

#	This software is supplied to AT & T under the terms of a license
#	agreement with Intel Corporation and may not be copied nor
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mb1:uts/i386/boot/mb1/mb1.mk	1.3"

# This makefile uses the 386 sgs executed under Unix System V/386 Release 3.2
# Before running the make, load the 386 sgs under a clean directory
# and set the following make macros accordingly. These macros are passed down
# from above, so they may not need to be modified here unless make in invoked
# directly in this directory.

CPP		= /lib/cpp
CCSTYPE	= ELF
ROOT	= 
INCRT	= ../..
INCS	= $(ROOT)/usr/include
MORECPP	= -D$(BUS)
INSDIR	= $(ROOT)/etc

CFLAGS	= -O -I$(INCRT) -I$(INCS) $(MORECPP)
LDFLAGS	=

ASRC	= bboot.s touchmem.s common.s prot.s msa.s
CSRC	= bload.c memsize.c i214.c i8251.c printf.c
SRCS	= $(ASRC) $(CSRC)
# 
# the order of the files is important.  For disk boot, bboot.o must be
# first, for tape boot, tapeboot.o must be first.
#
DOBJ	= $(ASRC:.s=.o) $(CSRC:.c=.o)
TOBJ	= # tapeboot.o tpboot.o prot.o common.o memsize.o touchmem.o string.o

all: dboot # tboot

lint: $(CSRC) llib-lboot.c
	lint -n $(CFLAGS) $(CSRC) llib-lboot.c |tee $@+
	mv $@+ $@

dboot: $(DOBJ)
	if [ x${CCSTYPE} = xELF ]; then \
		$(LD) $(LDFLAGS) -Mmapfile -dn -e bootstrap -o $@ $(DOBJ) ; \
	else \
		$(LD) $(LDFLAGS) -e bootstrap -o $@ $(DOBJ) ifile ; \
	fi 
	sh checkbss $@

tboot: $(TOBJ) $(OBJ)
	if [ x${CCSTYPE} = xELF ]; then \
		$(LD) $(LDFLAGS) -Mmapfile -dn -e bootstrap -o $@ $(TOBJ) $(OBJ) ; \
	else \
		$(LD) $(LDFLAGS) -e bootstrap -o $@ $(TOBJ) $(OBJ) ifile ; \
	fi 
	sh checkbss $@

.c.o:
	$(CC) -S $(CFLAGS) $*.c
	sed -e 's/\.data1,"aw"/.text/' -e 's/\.data1/.text/' \
		-e 's/\.data/.text/' $*.s >tmp.s
	$(AS) -o $@ tmp.s
	rm $*.s tmp.s

.s.o:
	$(CPP) $(MORECPP) -I$(INCRT) -I$(INCS) -P $? $*.i
	$(AS) -o $@ $*.i
	rm $*.i

clean:
	rm -f $(OBJ) $(TOBJ) $(DOBJ)

clobber: clean  
	rm -f dboot tboot

install: all
	install -f  $(INSDIR) -m 0644 -u root -g sys dboot 
#	install -f  $(INSDIR) -m 0644 -u root -g sys tboot 


bload.o: ../sys/blb.h ../sys/dib.h ../sys/boot.h ../sys/prot.h
i214.o: ../sys/farcall.h
i8251.o: ../sys/boot.h ../sys/farcall.h
memsize.o: ../sys/boot.h
