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

#ident	"@(#)kern-ml:ml.mk	1.3.1.1"

INCRT=..
CPPFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP) 
CFLAGS = -O $(CPPFLAGS)
ODIR=$(CONF)/pack.d/kernel
FRC =


SFILES = ttrap.s cswitch.s misc.s intr.s weitek.s v86gptrap.s oemsup.s \
	 string.s 

all:	tools $(ODIR)/locore.o $(ODIR)/start.o $(ODIR)/syms.o

tools:
	cd tool; make -f tools.mk

$(ODIR)/locore.o: locore.o tables1.o pit.o pic.o $(FRC)
	$(LD) -r -o $(ODIR)/locore.o tables1.o locore.o pit.o pic.o

locore.o:	$(SFILES) symvals.s
	echo '\t.file\t"locore.s"' >temp.c
	cat symvals.s $(SFILES) >>temp.c
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		$(CPP) $(CPPFLAGS) -DLOCORE -P temp.c temp.i ; \
	else \
		$(CC) $(CPPFLAGS) -DLOCORE -P temp.c ; \
	fi
	mv temp.i locore.s
	$(CC) -c locore.s
	/bin/rm -f locore.s temp.c

$(ODIR)/syms.o:	syms.o
	cp syms.o $@

$(ODIR)/start.o:	start.o
	cp start.o $@

#  syms.s must be the first to link into the kernel.

syms.o:	syms.s
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		$(CPP) $(CPPFLAGS) -DLOCORE -P syms.s syms.i ; \
	else \
		$(CC) $(CPPFLAGS) -DLOCORE -P syms.s ; \
	fi
	mv syms.i tmp.s
	$(CC) -c tmp.s
	mv tmp.o syms.o

start.o:	uprt.s tables2.o symvals.s $(FRC)
	echo '\t.file\t"uprt.s"' >temp.c
	cat symvals.s uprt.s >>temp.c
	if [ x${CCSTYPE} = xELF ] ;\
	then \
		$(CPP) $(CPPFLAGS) -DLOCORE -P temp.c temp.i ; \
	else \
		$(CC) $(CPPFLAGS) -DLOCORE -P temp.c ; \
	fi
	mv temp.i temp.s
	$(CC) -c temp.s
	$(LD) -r -o start.o temp.o tables2.o
	/bin/rm -f temp.[cso]

tables2.o:	tables2.c
	$(CC) $(CFLAGS) -S tables2.c
	sed '/^	.data/c\
	.text\
	.align	8' tables2.s >temp.s
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		mv temp.s temp2.s ; \
		awk '  {if ($$1==".zero") { k=$$2 / 4; \
			for (i=1; i <= k; i++) print "\t.4byte\t0"; j=$$2 % 4; \
			for (i=1; i <= j; i++) print "\t.byte\t0" } else { print $0 }}' temp2.s > temp.s ; \
	fi
	$(CC) $(CFLAGS) -c temp.s
	mv temp.o tables2.o
	/bin/rm -f temp.s  temp2.s

clean:
	-rm -f *.o symvals.h symvals.s [0-9]* tables2.s
	-rm -f locore.s start.s temp.? temp2.?
	cd tool; make -f tools.mk clean

clobber:	clean
	-rm -f $(ODIR)/locore.o $(ODIR)/start.o $(ODIR)/syms.o
	cd tool; make -f tools.mk clobber

FRC:

#
# Header dependencies
#

tables1.o: tables1.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h

tables2.o: tables2.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h

pit.o: pit.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/pit.h

pic.o: pic.c \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/ipl.h \
	$(INCRT)/sys/pic.h

symvals.s: Gensymvals \
	$(INCRT)/sys/types.h    \
	$(INCRT)/sys/immu.h     \
	$(INCRT)/sys/fs/s5inode.h    \
	$(INCRT)/sys/tss.h      \
	$(INCRT)/sys/seg.h      \
	$(INCRT)/sys/signal.h   \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h     \
	$(INCRT)/sys/systm.h   \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h     \
	$(INCRT)/sys/errno.h   \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/param.h   \
	$(INCRT)/sys/proc.h	\
	$(INCRT)/sys/v86.h
	sh ./Gensymvals "CC=$(CC)" "CFLAGS=$(CFLAGS)"

ttrap.o: ttrap.s \
	$(INCRT)/sys/ipl.h \
	$(INCRT)/sys/pic.h \
	$(INCRT)/symvals.h \
	$(INCRT)/sys/reg.h

v86gptrap.o: v86gptrap.s \
	$(INCRT)/vm/faultcatch.h

string.o: string.s 
