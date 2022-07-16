#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libns:libns.mk	1.10.8.1"
# This makefile makes libns.a, which is the library for
# the name server library.
# NOTE: this library is not for general use.  It is put
# 	in /usr/lib ONLY for the convenience of the
#	commands that use it.
#
ROOT =
CC = cc
LIBDIR = .
INC = $(ROOT)/usr/include
USRLIB = $(ROOT)/usr/lib
LIBNAME = libns.a
LLIB = ns
LINTLIB = llib-l$(LLIB).ln
LOG=-DLOGGING -DLOGMALLOC
PROFILE=
DEBUG=
CFLAGS=-O -I $(INC) $(DEBUG) $(LOG) $(PROFILE)
SRC= ind_data.c nsblock.c nsports.c nsrports.c \
	rtoken.c astoa.c stoa.c ns_comm.c nslog.c canon.c  \
	logmalloc.c ns_findp.c ns_getaddr.c ns_getblock.c ns_initaddr.c \
	ns_verify.c ns_error.c ns_errlist.c ns_dfinfo.c ns_info.c ns_sendpass.c \
	attconnect.c rfrequest.c negotiate.c getoken.c netname.c \
	swtab.c uidmap.c ns_syntax.c rfs_up.c rfrcv.c

FILES =\
	$(LIBNAME)(ind_data.o)\
	$(LIBNAME)(nsblock.o)\
	$(LIBNAME)(nsports.o)\
	$(LIBNAME)(nsrports.o)\
	$(LIBNAME)(rtoken.o)\
	$(LIBNAME)(stoa.o)\
	$(LIBNAME)(astoa.o)\
	$(LIBNAME)(ns_comm.o) \
	$(LIBNAME)(nslog.o) \
	$(LIBNAME)(canon.o) \
	$(LIBNAME)(logmalloc.o)\
	$(LIBNAME)(ns_getaddr.o)\
	$(LIBNAME)(ns_findp.o)\
	$(LIBNAME)(ns_getblock.o)\
	$(LIBNAME)(ns_initaddr.o)\
	$(LIBNAME)(ns_verify.o)\
	$(LIBNAME)(ns_error.o)\
	$(LIBNAME)(ns_errlist.o)\
	$(LIBNAME)(ns_dfinfo.o)\
	$(LIBNAME)(ns_info.o)\
	$(LIBNAME)(ns_sendpass.o)\
	$(LIBNAME)(attconnect.o)\
	$(LIBNAME)(rfrequest.o)\
	$(LIBNAME)(negotiate.o)\
	$(LIBNAME)(getoken.o)\
	$(LIBNAME)(netname.o)\
	$(LIBNAME)(uidmap.o)\
	$(LIBNAME)(rfs_up.o)\
	$(LIBNAME)(ns_syntax.o)\
	$(LIBNAME)(swtab.o)\
	$(LIBNAME)(rfrcv.o)

lib:	$(LIBNAME) 
debug:
	make -f libns.mk LIBNAME=libnsdb.a DEBUG="-g -DDEBUG -DLOGGING -DLOGMALLOC" lib
lint:
	lint -uax -DLOGGING -o $(LLIB) $(SRC)
install: lib
	cp $(LIBNAME) $(USRLIB)
uninstall:
	-rm $(USRLIB)/$(LIBNAME)

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBNAME)

#### dependencies now follow

$(LIBNAME)(nsports.o): stdns.h nsports.h nsdb.h $(INC)/nsaddr.h nslog.h
$(LIBNAME)(nsrports.o): stdns.h nsports.h nsdb.h $(INC)/nsaddr.h nslog.h \
	$(INC)/pn.h
$(LIBNAME)(rtoken.o): stdns.h nsdb.h $(INC)/sys/types.h $(INC)/sys/nserve.h\
	$(INC)/sys/rf_sys.h
$(LIBNAME)(ind_data.o): stdns.h nslog.h
$(LIBNAME)(nsblock.o): nslog.h nsdb.h stdns.h $(INC)/nserve.h
$(LIBNAME)(ns_comm.o): $(INC)/nserve.h $(INC)/nsaddr.h nslog.h stdns.h\
	nsports.h $(INC)/sys/types.h $(INC)/sys/nserve.h $(INC)/sys/rf_sys.h
$(LIBNAME)(nslog.o): nslog.h
$(LIBNAME)(astoa.o): $(INC)/nsaddr.h
$(LIBNAME)(stoa.o): $(INC)/nsaddr.h
$(LIBNAME)(ns_getaddr.o): $(INC)/nserve.h $(INC)/nsaddr.h
$(LIBNAME)(ns_findp.o): $(INC)/nserve.h $(INC)/nsaddr.h
$(LIBNAME)(ns_getblock.o): $(INC)/nserve.h
$(LIBNAME)(ns_initaddr.o): $(INC)/nserve.h
$(LIBNAME)(ns_verify.o): $(INC)/nserve.h
$(LIBNAME)(ns_sendpass.o): $(INC)/nserve.h
$(LIBNAME)(attconnect.o): $(INC)/pn.h
$(LIBNAME)(rfrequest.o): $(INC)/pn.h
$(LIBNAME)(negotiate.o): $(INC)/pn.h $(INC)/sys/types.h $(INC)/sys/nserve.h\
	$(INC)/sys/rf_sys.h
$(LIBNAME)(getoken.o): $(INC)/sys/nserve.h $(INC)/sys/rf_cirmgr.h
$(LIBNAME)(netname.o): $(INC)/string.h $(INC)/errno.h $(INC)/sys/nserve.h\
	$(INC)/sys/utsname.h $(INC)/sys/types.h $(INC)/sys/rf_sys.h
$(LIBNAME)(swtab.o): $(INC)/sys/nserve.h $(INC)/sys/rf_cirmgr.h\
	$(INC)/sys/param.h $(INC)/pn.h
$(LIBNAME)(uidmap.o): idload.h $(INC)/sys/types.h $(INC)/sys/rf_sys.h\
	$(INC)/errno.h $(INC)/nserve.h $(INC)/sys/param.h
$(LIBNAME)(rfs_up.o): $(INC)/nserve.h $(INC)/sys/types.h $(INC)/sys/nserve.h\
	$(INC)/sys/list.h $(INC)/sys/vnode.h $(INC)/sys/rf_messg.h\
	$(INC)/sys/rf_comm.h $(INC)/errno.h $(INC)/sys/rf_sys.h\
	$(INC)/stdio.h nslog.h
$(LIBNAME)(ns_syntax.o): $(INC)/nserve.h
$(LIBNAME)(rfrcv.o): $(INC)/tiuser.h
