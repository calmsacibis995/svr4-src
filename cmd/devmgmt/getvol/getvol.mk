#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devmgmt:getvol/getvol.mk	1.11.4.1"



PROC=getvol
SRC=getvol.c
OBJ=$(SRC:.c=.o)

## default paramter definitions
INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
CFLAGS=-O

## libraries used by this process
LIBPKG=$(USRLIB)/libpkg.a
LIBADM=$(USRLIB)/libadm.a
LINTLIBS=$(USRLIB)/llib-lpkg.ln $(USRLIB)/llib-ladm.ln

## options used to build this command
LIBOPTS=-ladm -lpkg
HDROPTS=-I. -I $(INC)
LDFLAGS=-s

## process build rules
.c.o:
	$(CC) -c $(HDROPTS) $(CFLAGS) $<

all:	$(PROC) 

$(PROC): $(OBJ) $(LIBINST) $(LIBPKG)
	$(CC) -o $(PROC) $(LDFLAGS) $(OBJ) $(LDLIBPATH) $(LIBOPTS)
	chmod 775 $(PROC)

OBJECTS=$(PROC)
PROTO=../Prototype
INSTALL=install
install: all
	@eval `sed -e '/^![^=]*=/!d' -e 's/^!//' $(PROTO)` ;\
	mkpath() { \
		while true ;\
		do \
			tmpdir=$$1 ;\
			[ -d $$tmpdir ] && break ;\
			while [ ! -d $$tmpdir ] ;\
			do \
				lastdir=$$tmpdir ;\
				tmpdir=`dirname $$tmpdir` ;\
			done ;\
			mkdir $$lastdir ;\
		done ;\
	} ;\
	for object in $(OBJECTS) ;\
	do \
		if entry=`grep "[ 	/]$$object[= 	]" $(PROTO)` ;\
		then \
			set -- $$entry ;\
			path=`eval echo $$3` ;\
			expr $$path : '[^/]' >/dev/null && \
				path=$(BASEDIR)/$$path ;\
			dir=$(ROOT)`dirname $$path` ;\
			[ ! -d $$dir ] && mkpath $$dir ;\
			$(INSTALL) -f $$dir -m $$4 -u $$5 -g $$6 $$object ;\
		else \
			echo "unable to install $$object" ;\
		fi ;\
	done

clobber: clean
	rm -f $(PROC)

clean:
	rm -f $(OBJ)

strip:
	$(CC) $(CFLAGS) $(PROC).o -o $(PROC) $(LDLIBPATH) $(CFLAGS) 

linti:
	lint $(SRC) $(HDROPTS) $(LINTLIBS) $$i >lint.out 2>&1

HDRS=\
	$(INC)/stdio.h $(INC)/string.h $(INC)/errno.h \
	$(INC)/fmtmsg.h $(INC)/devmgmt.h $(INC)/signal.h $(INC)/sys/types.h
$(OBJ): $(HDRS)
