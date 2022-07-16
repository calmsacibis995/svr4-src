#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-fp:fp.mk	1.1.1.1"

#	===============================================================
#               intel corporation proprietary information
#    this software  is  supplied  under  the  terms  of  a  license
#    agreement  or non-disclosure agreement  with intel corporation
#    and  may not be copied  or disclosed except in accordance with
#    the terms of that agreement.                                  
#	===============================================================

INSDIR = $(ROOT)/sbin
INS = install -f $(INSDIR)
CPPFLAGS = $(MORECPP)

OBJ= dcode.o arith.o divmul.o lipsq.o reg.o remsc.o round.o status.o  \
	store.o subadd.o trans.o

all:	tools emulator.rel1

tools:
	cd tool; make -f tools.mk

emulator.rel1: $(OBJ) 
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		$(LD) -Mmapfile -dn -s -e e80387 -o $@ $(OBJ) ; \
	else \
		$(LD) -s -e e80387 -o $@ vuifile $(OBJ) ; \
	fi

install: all
	$(INS) emulator.rel1 $(INSDIR) 744 bin bin
	-ln $(INSDIR)/emulator.rel1 $(INSDIR)/emulator
	-ln $(INSDIR)/emulator $(ROOT)/etc/emulator
	-ln $(INSDIR)/emulator.rel1 $(ROOT)/etc/emulator.rel1

clean:
	-rm -f $(OBJ) symvals.h
	-rm emulator.rel1
	cd tool; make -f tools.mk clean

clobber: clean
	-rm $(INSDIR)/emulator
	-rm $(INSDIR)/emulator.rel1
	cd tool; make -f tools.mk clobber

$(OBJ):	symvals.h e80387.h

.s.o:
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		$(CPP) $(CPPFLAGS) -P $< $*.i ;\
	else \
		$(CC) $(CPPFLAGS) -P $< ;\
	fi
	mv $*.i tmp.s
	$(CC) -c tmp.s 
	mv tmp.o $*.o
	rm tmp.s

symvals.h: Gensymvals \
	$(INCRT)/sys/seg.h    \
	$(INCRT)/sys/types.h    \
	$(INCRT)/sys/immu.h     \
	$(INCRT)/sys/tss.h      \
	$(INCRT)/sys/seg.h      \
	$(INCRT)/sys/signal.h   \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h     
	sh ./Gensymvals "CC=$(CC)" "CFLAGS=$(CFLAGS)"
