#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rtld:i386/makefile	1.8"

# makefile for i386 version of the run-time linker

SGSINC=$(SGSBASE)/inc
MODE= -Kpic
COMDIR=../common
MACH=i386
MACHDIR=../$(MACH)
DEFLIST=-D$(MACH) -DWEITEK
INCLIST= -I$(COMDIR) -I$(MACHDIR) -I$(SGSINC)/common -I$(SGSINC)/$(MACH)
PROPTS= -dxerox -omode=l -omode=h
AWK=awk
ECHO=echo

HFILES= $(COMDIR)/rtld.h $(COMDIR)/dllib.h $(COMDIR)/externs.h \
	$(MACHDIR)/machdep.h $(COMDIR)/rtinc.h

SOURCES= $(COMDIR)/dlfcns.c $(COMDIR)/globals.c $(COMDIR)/map.c \
	 $(COMDIR)/rtfcns.c $(COMDIR)/rtld.c $(COMDIR)/paths.c  \
	 $(MACHDIR)/reloc.c $(MACHDIR)/rtsetup.c $(MACHDIR)/binder.c 

AS_SOURCES= $(MACHDIR)/rtbinder.s $(MACHDIR)/rtboot.s $(MACHDIR)/rtmemcpy.s \
	 $(MACHDIR)/clrpage.s $(MACHDIR)/align.s

# rtboot.o, which contains the startup code, must be linked first
OBJECTS= align.o rtboot.o binder.o dlfcns.o globals.o map.o paths.o reloc.o \
	rtfcns.o rtld.o rtsetup.o rtbinder.o clrpage.o rtsetaddr.o rtmemcpy.o \
	rtabiaddr.o

PRODUCTS= $(OBJECTS)


all:	$(OBJECTS) 

dlfcns.o:	$(COMDIR)/dlfcns.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/dlfcns.c

globals.o:	$(COMDIR)/globals.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/globals.c

map.o:	$(COMDIR)/map.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/map.c

rtfcns.o:	$(COMDIR)/rtfcns.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/rtfcns.c

rtld.o:	$(COMDIR)/rtld.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/rtld.c

paths.o:	$(COMDIR)/paths.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(COMDIR)/paths.c

binder.o:	$(MACHDIR)/binder.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACHDIR)/binder.c

rtsetup.o:	$(MACHDIR)/rtsetup.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACHDIR)/rtsetup.c

reloc.o:	$(MACHDIR)/reloc.c $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACHDIR)/reloc.c

rtboot.o:	$(MACHDIR)/rtboot.s $(HFILES)
	$(CC) -c $(CFLAGS) $(MODE) $(MACHDIR)/rtboot.s

rtbinder.o:	$(MACHDIR)/rtbinder.s $(HFILES)
	$(CC) -Wa,-m -c $(CFLAGS) $(MODE) $(MACHDIR)/rtbinder.s

clrpage.o:	$(MACHDIR)/clrpage.s
	$(CC) -c $(CFLAGS) $(MODE) $(MACHDIR)/clrpage.s

rtmemcpy.o:	$(MACHDIR)/rtmemcpy.s
	$(CC) -c $(CFLAGS) $(MODE) $(MACHDIR)/rtmemcpy.s

rtsetaddr.o:	$(MACHDIR)/genset.awk $(MACHDIR)/genset.in $(HFILES)
	$(ECHO) "@FULL" | $(AWK) -f $(MACHDIR)/genset.awk - $(MACHDIR)/genset.in \
		> $(MACHDIR)/rtsetaddr.s
	$(CC) -c $(CFLAGS) $(MODE) $(MACHDIR)/rtsetaddr.s

rtabiaddr.o:	$(MACHDIR)/genset.awk $(MACHDIR)/genset.in $(HFILES)
	$(ECHO) "@MIN" | $(AWK) -f $(MACHDIR)/genset.awk - $(MACHDIR)/genset.in \
		> $(MACHDIR)/rtabiaddr.s
	$(CC) -c $(CFLAGS) $(MODE) $(MACHDIR)/rtabiaddr.s


lintit:	$(SOURCES) $(HFILES)
	$(LINT) $(LINTFLAGS) $(INCLIST) $(DEFLIST) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

clobber:	clean
	-rm -f rtsetaddr.s rtabiaddr.s

print:	$(SOURCES) $(AS_SOURCES) $(HFILES)
	pr -n $(SOURCES) $(AS_SOURCES) $(HFILES) | lp $(PROPTS) 
