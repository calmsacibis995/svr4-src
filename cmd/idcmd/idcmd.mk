#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#ident	"@(#)idcmd:idcmd.mk	1.3"
#
# Makefile to build and install Driver Installation commands.

INCRT = $(ROOT)/usr/include
CONF=$(ROOT)/etc/conf
CONFDIR = $(CONF)
INSDIR = $(CONFDIR)/bin
ROOTLIBS = -dn
CFLAG = -O
CMD1 = idconfig idmkunix idmaster idinstall idmkinit idmknod idcheck idspace idmkenv idval
CMD2 = idbuild idreboot idtune
BUS = AT386
ARCH=AT386
MORECPP = -D$(BUS) $(DFLGS) -DWEITEK -DMERGE -D$(ARCH)
DFLGS = -DVPIX
NET=
NET2=
NET3=

all:	$(CMD1) $(CMD2)

install: all 
	if [ ! -d $(ROOT)/etc ] ;\
	then \
		mkdir $(ROOT)/etc ;\
	fi
	if [ ! -d $(ROOT)/etc/idrc.d ] ;\
	then \
		mkdir $(ROOT)/etc/idrc.d ;\
	fi
	if [ ! -d $(ROOT)/etc/idsd.d ] ;\
	then \
		mkdir $(ROOT)/etc/idsd.d ;\
	fi
	if [ ! -d $(CONFDIR) ] ;\
	then \
		mkdir $(CONFDIR) ;\
	fi
	if [ ! -d $(INSDIR) ] ;\
	then \
		mkdir $(INSDIR) ;\
	fi

	chmod 744 $(CMD1) $(CMD2)
	cp $(CMD1) $(CMD2) $(INSDIR)
	- if expr "`cc -V 2>&1`" : '.*CDE..5.0.*' > /dev/null; then : ; else install -f $(INSDIR) idconfig_native ; \
			 install -f $(INSDIR) idmkunix_native ; \
	fi

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(CMD1) idmkunix_native idconfig_native

idconfig: idconfig.c \
	defines.h \
	inst.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/stdio.h \
	$(INCRT)/time.h \
	$(INCRT)/ctype.h \
	$(INCRT)/string.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idconfig.c -o idconfig
	- if expr "`cc -V 2>&1`" : '.*CDE..5.0.*' > /dev/null; then : ; else rm -f idconfig.o ; \
			mv idconfig idconfig_native ; \
			cc -D_STYPES -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idconfig.c -o idconfig ; \
	fi

idmkunix: idmkunix.c \
	inst.h \
	$(INCRT)/stdio.h \
	$(INCRT)/string.h \
	$(INCRT)/ctype.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stat.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idmkunix.c -o idmkunix
	- if expr "`cc -V 2>&1`" : '.*CDE..5.0.*' > /dev/null; then : ; else rm -f idmkunix.o ; \
			mv idmkunix idmkunix_native ; \
			cc -D_STYPES -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idmkunix.c -o idmkunix ; \
	fi

idinstall: idinstall.c \
	$(INCRT)/stdio.h \
	$(INCRT)/filehdr.h \
	$(INCRT)/ctype.h \
	$(INCRT)/string.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/ustat.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s idinstall.c -o idinstall

idmaster: idmaster.c \
	getmajors.o \
	getinst.o \
	inst.h \
	defines.h \
	$(INCRT)/stdio.h \
	$(INCRT)/ctype.h \
	$(INCRT)/string.h \
	$(INCRT)/signal.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s getmajors.o getinst.o idmaster.c -o idmaster

idcheck: idcheck.c \
	getmajors.o \
	getinst.o \
	inst.h \
	defines.h \
	$(INCRT)/stdio.h \
	$(INCRT)/ctype.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stat.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s getmajors.o getinst.o idcheck.c -o idcheck

idmknod: idmknod.c \
	getmajors.o \
	getinst.o \
	inst.h \
	$(INCRT)/stdio.h \
	$(INCRT)/string.h \
	$(INCRT)/ctype.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/errno.h \
	$(INCRT)/sys/stat.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s getmajors.o getinst.o idmknod.c -o idmknod

idmkinit: idmkinit.c \
	inst.h \
	$(INCRT)/stdio.h \
	$(INCRT)/string.h \
	$(INCRT)/ctype.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dir.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s idmkinit.c -o idmkinit

idspace: idspace.c \
	$(INCRT)/stdio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/statvfs.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/mnttab.h \
	$(INCRT)/ustat.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idspace.c -o idspace

idmkenv: idmkenv.c \
	inst.h \
	$(INCRT)/stdio.h \
	$(INCRT)/string.h \
	$(INCRT)/fcntl.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/pwd.h \
	$(INCRT)/grp.h \
	$(INCRT)/varargs.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s idmkenv.c -o idmkenv

getinst.o: getinst.c \
	getmajors.o \
	inst.h \
	$(INCRT)/stdio.h \
	$(INCRT)/ctype.h
	$(CC) -D$(BUS) -c -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) -s getmajors.o getinst.c

getmajors.o: getmajors.c \
	defines.h \
	inst.h \
	$(INCRT)/stdio.h
	$(CC) -D$(BUS) -c -I$(INCRT) -I "." $(CFLAG) $(MORECPP) $(ROOTLIBS) getmajors.c

idval: idval.c \
	$(INCRT)/stdio.h
	$(CC) -D$(BUS) -I$(INCRT) -I "." $(CFLAG) $(MORECPP) -s idval.c -o idval
