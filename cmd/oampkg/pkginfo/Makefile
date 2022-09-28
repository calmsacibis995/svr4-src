#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oampkg:pkginfo/pkginfo.mk	1.6.4.1"
.SUFFIXES:	.df

SRCBASE=../
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
LINTLIBS=$(USRLIB)/llib-lpkg.ln $(USRLIB)/llib-ladm.ln
CFLAGS=-O
LDFLAGS=-s

.sh:
	cp $< $*
	chmod 775 $*

.c:
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -I $(INC) $@.c $(LDLIBPATH) -lpkg -ladm
	chmod 775 $@

O_SHFILES=
O_CFILES=pkginfo pkgparam pkgname
OBJECTS=$(O_CFILES) $(O_SHFILES)

all: .check $(OBJECTS)
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
