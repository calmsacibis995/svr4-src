#ident	"@(#)lib:libmk.template	1.15"
#	Template for Global Library Makefile
#
#
CCSLIB=$(ROOT)/usr/ccs/lib
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LIBP=$(ROOT)/usr/ccs/lib/libp
OWN=bin
GRP=bin
CCSBIN=$(ROOT)/usr/ccs/bin
LIBLIST=*
MACH=i386
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
MAKE=make
SGS=
MAC=
CFLAGS= -O

all:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	echo $$i; cd $$i; $(MAKE) -f $$i.mk MAC=$(MAC) CFLAGS="$(CFLAGS)"; cd ..; \
	   fi \
	done
	if u3b2 || u3b5 || u3b15 ; \
	then \
		cd libc; $(MAKE) -f libc.mk clobber; \
		$(MAKE) -f libc.mk MAC=$(MAC) CFLAGS="-O -K mau" archive; \
	fi

install:
	cd libc; $(MAKE) -f libc.mk clobber
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
		echo $$i; \
	   	cd $$i; \
	   	$(MAKE) -f $$i.mk CCSLIB=$(CCSLIB) LIBP=$(LIBP) LIB=$(LIB) USRLIB=$(USRLIB) SGS=$(SGS) MAC=$(MAC) CFLAGS="$(CFLAGS)" OWN=$(OWN) GRP=$(GRP) MACH="$(MACH)" install; \
	   	cd ..; \
	   fi \
	done
	if u3b2 || u3b5 || u3b15 ; \
	then \
		cd libc; $(MAKE) -f libc.mk clobber; \
		$(MAKE) -f libc.mk CCSLIB=$(CCSLIB)/fp LIBP=$(CCSLIB)/fp/libp LIB=$(LIB) USRLIB=$(USRLIB) SGS=$(SGS) MAC=$(MAC) CFLAGS="-O -K mau" OWN=$(OWN) GRP=$(GRP) MACH="$(MACH)" install_archive; \
	fi

clean:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	echo $$i; cd $$i; $(MAKE) -f $$i.mk clean; cd ..; \
	   fi \
	done

clobber:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	echo $$i; cd $$i; $(MAKE) -f $$i.mk clobber; cd ..; \
	   fi \
	done

lintit:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	echo $$i; cd $$i; $(MAKE) -f $$i.mk lintit; cd ..; \
	   fi \
	done
