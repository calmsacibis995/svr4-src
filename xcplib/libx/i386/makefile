#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xcplibx:i386/makefile	1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#
# makefile for libx/i386
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

.SUFFIXES: .p

M4=m4
CC=cc
CFLAGS=-O -DMERGE
PROF=
NONPROF=
INCLIBX=../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBX)
M4DEFS=m4.def sys.def

POBJECTS=$(OBJECTS:.o=.p)

OBJECTS= \
sys/chsize.o \
sys/creatsem.o \
sys/execseg.o \
sys/ftime.o \
sys/locking.o \
sys/nap.o \
sys/nbwaitsem.o \
sys/opensem.o \
sys/proctl.o \
sys/rdchk.o \
sys/sdfree.o \
sys/_sdget.o \
sys/sdgetv.o \
sys/sdenter.o \
sys/sdleave.o \
sys/sdwaitv.o \
sys/sigsem.o \
sys/unexecseg.o \
sys/waitsem.o

all:
	$(PROF)make -e $(IGN) prof NONPROF=@#
	$(NONPROF)make -e $(IGN) nonprof PROF=@#

nonprof: $(OBJECTS) 

prof: $(POBJECTS) 

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS)  -c $*.c
	$(NONPROF)mv $(*F).o $*.o
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p

.s.o .s.p:
	$(NONPROF)@echo $*.s:
	$(NONPROF)$(M4) $(M4DEFS) $(SDEFLIST) -DMCOUNT=/  $*.s   > $(*F).s   
	$(NONPROF)$(CC) $(DEFLIST) $(CFLAGS) -c $(*F).s && mv $(*F).o $*.o
	$(PROF)@echo $*.s:
	$(PROF)$(M4) $(M4DEFS) $(SDEFLIST) mcount.def $*.s   > $(*F).s 
	$(PROF)$(CC) $(DEFLIST) $(CFLAGS) -c -p $(*F).s  && mv $(*F).o $*.p
	-rm $(*F).s

archive:
	#
	# Note that "archive" is invoked with libx/object as current directory.
	#
	# figure out the correct ordering for all the archive modules 
	$(LORDER) *.o | $(PFX)tsort >objlist
	#
	# build the archive with the modules in correct order.
	$(AR) q ../lib.libx `cat objlist` 
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
	if [ "$(PROF)" != "@#" ]; \
	then \
	$(PROF)$(AR) q ../libp.libx `cat objlist` ; \
	fi

clean:
	-rm -f *.o
	-rm -f *.p
	-rm -f *.s

clobber: clean
	-rm -f */*.o
	-rm -f */*.p

