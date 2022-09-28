#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.sbin/in.named/in.named.mk	1.18.4.1"

#
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# 		PROPRIETARY NOTICE (Combined)
# 
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
# 
# 
# 
# 		Copyright Notice 
# 
# Notice of copyright on this source code product does not indicate 
# publication.
# 
# 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
# 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
# 	          All rights reserved.
#  
#

DASHO=		-O
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP
LDFLAGS=	-s
INC=		$(ROOT)/usr/include
USRSBIN=	$(ROOT)/usr/sbin
INSTALL=	install

# want debugging to be on so "-d" option works
CFLAGS=		$(DASHO) $(MORECPP) -DDEBUG -I$(INC)


LIBS=		-lresolv -lsocket -lnsl $(SHLIBS)

OBJS=		db_dump.o db_load.o db_lookup.o db_reload.o db_save.o \
		db_update.o ns_forw.o ns_init.o ns_main.o ns_maint.o \
		ns_req.o ns_resp.o ns_sort.o version.o



PASSECHO=	\"AR=$(AR)\" \"AS=$(AS)\" \"CC=$(CC)\" \"DASHO=$(DASHO)\" \
		\"FRC=$(FRC)\"  \"INC=$(INC)\" \
		\"INSTALL=$(INSTALL)\" \"LD=$(LD)\" \"LDFLAGS=$(LDFLAGS)\" \
		\"LEX=$(LEX)\" \"LIB_OPT=$(LIB_OPT)\" \"MAKE=$(MAKE)\" \
		\"MAKEFLAGS=$(MAKEFLAGS)\" \
		\"MORECPP=$(MORECPP)\" \"ROOT=$(ROOT)\"  \
		\"YACC=$(YACC)\" \"SHLIBS=$(SHLIBS)\" \"SYMLINK=$(SYMLINK)\"

PASSTHRU=	"AR=$(AR)" "AS=$(AS)" "CC=$(CC)" "DASHO=$(DASHO)" \
		"FRC=$(FRC)" "INC=$(INC)" \
		"INSTALL=$(INSTALL)" "LD=$(LD)" "LDFLAGS=$(LDFLAGS)" \
		"LEX=$(LEX)" "LIB_OPT=$(LIB_OPT)" "MAKE=$(MAKE)" \
		"MAKEFLAGS=$(MAKEFLAGS)" \
		"MORECPP=$(MORECPP)" "ROOT=$(ROOT)"  \
		"YACC=$(YACC)" "SHLIBS=$(SHLIBS)" "SYMLINK=$(SYMLINK)"


all:		in.named tooldir

in.named:	$(OBJS)
		$(CC) -o in.named $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

tooldir:
		@cd tools;\
		echo "\n===== $(MAKE) -f tools.mk all $(PASSECHO)";\
		$(MAKE) -f tools.mk all $(PASSTHRU)

install:	in.named
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin in.named
		@cd tools;\
		echo "\n===== $(MAKE) -f tools.mk install $(PASSECHO)";\
		$(MAKE) -f tools.mk install $(PASSTHRU)

clobber:
		rm -f *.o in.named
		@cd tools;\
		echo "\n===== $(MAKE) -f tools.mk clobber $(PASSECHO)";\
		$(MAKE) -f tools.mk clobber $(PASSTHRU)

clean:
		rm -f *.o
		@cd tools;\
		echo "\n===== $(MAKE) -f tools.mk clean $(PASSECHO)";\
		$(MAKE) -f tools.mk clean $(PASSTHRU)

FRC:

#
# Header dependencies
#

db_dump.o:	db_dump.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stat.h \
		$(INC)/netinet/in.h \
		$(INC)/netdb.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

db_load.o:	db_load.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stat.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

db_lookup.o:	db_lookup.c \
		$(INC)/sys/types.h \
		$(INC)/stdio.h \
		$(INC)/arpa/nameser.h \
		db.h \
		$(FRC)

db_reload.o:	db_reload.c \
		$(INC)/sys/types.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

db_save.o:	db_save.c \
		$(INC)/sys/types.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		db.h \
		$(FRC)

db_update.o:	db_update.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

gtdtablesize.o:	gtdtablesize.c \
		$(INC)/sys/resource.h \
		$(FRC)

ns_forw.o:	ns_forw.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_init.o:	ns_init.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/signal.h \
		$(INC)/syslog.h \
		$(INC)/ctype.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_main.o:ns_main.c \
		$(INC)/sys/param.h \
		$(INC)/fcntl.h \
		$(INC)/sys/file.h \
		$(INC)/sys/time.h \
		$(INC)/sys/wait.h \
		$(INC)/sys/resource.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/sockio.h \
		$(INC)/netinet/in.h \
		$(INC)/net/if.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/errno.h \
		$(INC)/signal.h \
		$(INC)/netdb.h \
		$(INC)/arpa/nameser.h \
		$(INC)/arpa/inet.h \
		$(INC)/resolv.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_maint.o:	ns_maint.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/unistd.h \
		$(INC)/utime.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/syslog.h \
		$(INC)/signal.h \
		$(INC)/errno.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_req.o:		ns_req.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/uio.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/netinet/in.h \
		$(INC)/syslog.h \
		$(INC)/sys/file.h \
		$(INC)/arpa/nameser.h \
		$(INC)/fcntl.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_resp.o:	ns_resp.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/file.h \
		$(INC)/netinet/in.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

ns_sort.o:	ns_sort.c \
		$(INC)/stdio.h \
		$(INC)/sys/types.h \
		$(INC)/sys/time.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/file.h \
		$(INC)/netinet/in.h \
		$(INC)/syslog.h \
		$(INC)/arpa/nameser.h \
		ns.h \
		$(INC)/string.h \
		$(INC)/arpa/inet.h \
		db.h \
		$(FRC)

version.o:	version.c \
		$(FRC)
