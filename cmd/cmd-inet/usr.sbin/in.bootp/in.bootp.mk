# 	Copyrighted as an unpublished work.
#       (c) Copyright 1990 INTERACTIVE Systems Corporation
#       All Rights Reserved.
#
#	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
#	All Rights Reserved.
#
#	The copyright above and this notice must be preserved in all
#	copies of this source code.  The copyright above does not
#	evidence any actual or intended publication of this source
#	code.
#
#	This is unpublished proprietary trade secret source code of
#	Lachman Associates.  This source code may not be copied,
#	disclosed, distributed, demonstrated or licensed except as
#	expressly authorized by Lachman Associates.
#
#	System V STREAMS TCP was jointly developed by Lachman
#	Associates and Convergent Technologies.
#
#

DASHO=		-O
CONFFILE=	-DCONFIG_FILE=\"/etc/inet/bootptab\"
DUMPFILE=	-DDUMP_FILE=\"/etc/inet/bootpd.dump\"
FILESPEC=	$(CONFFILE) $(DUMPFILE)
BOOTPCPP=	-DSYSLOG -DDEBUG -DVEND_CMU -DPRIVATE=static $(FILESPEC)
MORECPP=	-DSYSV -DSTRNET -DBSD_COMP -DBSD=43
INC=		$(ROOT)/usr/include
USRSBIN=	$(ROOT)/usr/sbin
INET=		$(ROOT)/etc/inet
INSTALL=	install
FRC=

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC) $(BOOTPCPP)

LIBS=		-lsocket -lnsl $(SHLIBS)

OBJS=		bootpd.o readfile.o hash.o

all:		in.bootpd

in.bootpd:	$(OBJS)
		$(CC) -o in.bootpd  $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

install:	$(USRSBIN)/in.bootpd $(INET)/bootptab

$(USRSBIN)/in.bootpd: in.bootpd
		$(INSTALL) -f $(USRSBIN) -m 0555 -u bin -g bin in.bootpd

$(INET)/bootptab: bootptab
		$(INSTALL) -f $(INET) -m 0444 -u root -g sys bootptab

clean:
		rm -f $(OBJS) list.o a.out core errs

clobber:	clean
		rm -f in.bootpd

FRC:

#
# Header dependencies
#
bootpd.o:	bootpd.c \
		bootpd.h \
		bootp.h \
		hash.h \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/file.h \
		$(INC)/sys/fcntl.h \
		$(INC)/sys/time.h \
		$(INC)/sys/stat.h \
		$(INC)/net/if.h \
		$(INC)/sys/socket.h \
		$(INC)/net/if_arp.h \
		$(INC)/netinet/in.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/errno.h \
		$(INC)/errno.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/syslog.h \
		$(FRC)

readfile.o:	readfile.c \
		bootpd.h \
		bootp.h \
		hash.h \
		$(INC)/sys/types.h \
		$(INC)/sys/stat.h \
		$(INC)/sys/file.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/stdio.h \
		$(INC)/string.h \
		$(INC)/ctype.h \
		$(INC)/syslog.h \
		$(FRC)

hash.o:		hash.c \
		hash.h \
		$(FRC)
