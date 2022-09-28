#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oampkg:pkgscripts/pkgscripts.mk	1.4.5.1"
.SUFFIXES:	.df

SRCBASE=../
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LINTLIBS=$(USRLIB)/llib-lpkg.ln
OBJECTS=$(O_CFILES) $(O_SHFILES) $(O_DFFILES)

CFLAGS=-O
LDFLAGS=-s

.df:
	cp $< $*
	chmod 644 $*

.sh:
	cp $< $*
	chmod 775 $*

.c:
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) -I $(INC) $@.c $(LDLIBPATH) -lpkg
	chmod 775 $@

O_SHFILES=
O_CFILES=cmdexec
O_DFFILES=default r.build i.build i.sed i.awk r.sed r.awk

all: .check $(O_CFILES) $(O_SHFILES) $(O_DFFILES)
	@:

.check:	$(INC)/pkginfo.h
	rm -f $(O_CFILES)
	touch .check

clean:
	rm -f .check

clobber: clean
	rm -f $(O_CFILES) $(O_SHFILES) 

strip:
	$(STRIP) $(O_CFILES)

lintit:
	rm -f lint.out
	for file in $(O_CFILES) ;\
	do \
		lint -I $(INC) $$file.c $(LINTLIBS) >>lint.out ;\
	done

include $(SRCBASE)Install
