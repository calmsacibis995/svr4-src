#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:usr.bin/tftp/tftp.mk	1.15.3.1"

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
USRBIN=		$(ROOT)/usr/bin
INSTALL=	install

CFLAGS=		$(DASHO) $(MORECPP) -I$(INC)
LIBS=		-lsocket -lnsl $(SHLIBS)

PRODUCTS=	tftp

OBJ=		main.o tftp.o tftpsubs.o

all: $(PRODUCTS)


tftp:		$(OBJ)
		$(CC) -o tftp  $(CFLAGS) $(LDFLAGS) $(OBJ) $(LIBS)

install:	all
		$(INSTALL) -f $(USRBIN) -m 0555 -u bin -g bin tftp

clean:
		rm -f $(OBJ) core a.out

clobber:	clean
		rm -f $(PRODUCTS)

FRC:

#
# Header dependencies
#

main.o:		main.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/file.h \
		$(INC)/netinet/in.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/setjmp.h \
		$(INC)/ctype.h \
		$(INC)/netdb.h \
		$(INC)/fcntl.h \
		$(FRC)


tftp.o:		tftp.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/time.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/tftp.h \
		$(INC)/signal.h \
		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC)/setjmp.h \
		$(FRC)

tftpsubs.o:	tftpsubs.c \
		$(INC)/sys/types.h \
		$(INC)/sys/socket.h \
		$(INC)/sys/ioctl.h \
		$(INC)/sys/filio.h \
		$(INC)/netinet/in.h \
		$(INC)/arpa/tftp.h \
		$(INC)/stdio.h \
		$(INC)/tiuser.h \
		$(FRC)
