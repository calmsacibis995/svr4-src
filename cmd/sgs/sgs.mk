#ident	"@(#)sgs:sgs.mk.i386	1.4.4.1"
#	i386 Cross-SGS Global Makefile
#	PATHEDIT MUST BE RUN BEFORE THIS MAKEFILE IS USED!
#
#

YACC=yacc
LEX=lex
CC=cc
CCSBIN=
CCSLIB=/usr/ccs/lib
USRLIB=
SGS=
MAC=

all:	install libs
	echo "Finished building and installing both tools and libraries."

libs: 
	cd ../../lib; make -f .lib.mk install YACC="$(YACC)" LEX="$(LEX)" CC="$(CC)" MAC="$(MAC)" CCSBIN="$(CCSBIN)"
	cd xenv/i386; \
		 make libclobber; \
		 make libs; \
		 make libinstall; \
		 make libclobber;
		echo "Installed default libraries."

install:  sgs	
	cd xenv/i386; make CCSBIN=$(ROOT)/usr/ccs/bin CCSLIB=$(ROOT)/usr/ccs/lib install YACC="$(YACC)" LEX="$(LEX)" CC="$(CC)"
	cd xenv/i386; make libcopy

sgs:	
	cd xenv/i386; $(MAKE) all YACC="$(YACC)" LEX="$(LEX)" CC="$(CC)"

shrink:	clobber
	if [ true ] ; \
	then \
		cd ../../lib; make -f .lib.mk clobber ; \
	fi

lintit:
	cd xenv/i386; make lintit 

libslintit:
	cd ../../lib; make .lib.mk lintit

clean:
	cd xenv/i386; make clean

clobber:
	cd xenv/i386; make clobber
