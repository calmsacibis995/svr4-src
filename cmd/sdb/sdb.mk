#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:makefile	1.27.2.1"

include util/defs.make

ENVPARMS = \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" DFLAGS="$(DFLAGS)"\
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LINK_MODE="$(LINK_MODE)" \
	AR="$(AR)" COMINC="$(COMINC)" MACHINC="$(MACHINC)" LEX="$(LEX)" \
	INC="$(INC)" MAKE="$(MAKE)" YACC="$(YACC)" CPLUS="$(CPLUS)"

LIBDIRS = libdbgen libexecon libexp libint libmachine libsymbol libutil libC

PRODUCTS = sdb

DIRS = $(LIBDIRS) sdb.d

LIBS = lib/libdbgen.a lib/libexecon.a lib/libexp.a lib/libint.a \
	lib/libmachine.a lib/libsymbol.a lib/libutil.a

FORCE = force

all:	$(PRODUCTS)

$(PRODUCTS):	lib cfront/cfront $(LIBS) util/munch lib/libC.a $(FORCE)
	cd sdb.d/$(MACH) ; $(MAKE) $(ENVPARMS)

lib:
	mkdir lib

cfront/cfront:
	chmod +x util/depend util/Basename util/mkdefine util/substdir
	chmod 755 util/CC cfront/xCC
	cd cfront ; make scratch # native, not cross
	cd cfront ; make # native, not cross
#	if  uts ;	\
#	then 	\
#		chmod 755 util/CC ;	\
#		cp /star/usp/CC1.2/cfront  cfront/ ;	\
#		cp /star/usp/CC1.2/munch  cfront/ ;	\
#	fi
#	if  i386 ;	\
#	then	\
#		cp /usr/bin/cfront cfront/cfront ;	\
#		cp /usr/bin/munch cfront/munch ;	\
#		chmod 755 util/CC ;	\
#		#cp /usr/bin/CC  util/CC ;	\
#	fi

util/munch:	cfront/munch
	cp cfront/munch util/munch

lib/libC.a:
	cd libC/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libdbgen.a:	$(FORCE)
	cd libdbgen/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libexecon.a:	$(FORCE)
	cd libexecon/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libexp.a:	$(FORCE)
	cd libexp/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libint.a:	$(FORCE)
	cd libint/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libmachine.a:	$(FORCE)
	cd libmachine/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libsymbol.a:	$(FORCE)
	cd libsymbol/$(MACH) ; $(MAKE) $(ENVPARMS)

lib/libutil.a:	$(FORCE)
	cd libutil/$(MACH) ; $(MAKE) $(ENVPARMS)

install:	sdb
	cd sdb.d/$(MACH) ; $(MAKE) $(ENVPARMS) install

depend:	lib
	@for i in $(DIRS) ;\
	do echo $$i: ; ( cd $$i/$(MACH) ; $(MAKE) $(ENVPARMS) depend ) ;\
	done

clean:	
	@for i in $(DIRS) ;\
	do echo $$i: ; ( cd $$i/$(MACH) ; $(MAKE) $(ENVPARMS) clean ) ;\
	done
	cd cfront ; make clean
	rm -f sdb.d/$(MACH)/y.tab.h

clobber: 
	@for i in $(DIRS) ;\
	do echo $$i: ; ( cd $$i/$(MACH) ; $(MAKE) $(ENVPARMS) clobber ) ;\
	done
	cd cfront ; make clean
	rm -f cfront/libC.a cfront/munch cfront/xcfront cfront/cfront
	rm -f util/munch lib/libC.a sdb.d/$(MACH)/y.tab.h

lintit:
	@echo "can't lint C++"

rebuild:	clobber depend all

force:
	@:
