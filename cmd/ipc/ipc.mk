#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ipc:/ipcs.mk	1.7.8.4"

INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
LIST = lp
CFLAGS = -O -I$(INC)
OPTFLAGS = -D_KMEMUSER
LDFLAGS = -s -lelf
INS = install
FRC =

all:	ipcs ipcrm

install:	all
	$(INS) -f $(INSDIR) -m 2555 -u bin -g sys ipcs
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin ipcrm

ipcs:	$(INC)/sys/types.h\
	$(INC)/sys/ipc.h\
	$(INC)/sys/msg.h\
	$(INC)/sys/sem.h\
	$(INC)/sys/shm.h\
	$(INC)/a.out.h\
	$(INC)/fcntl.h\
	$(INC)/time.h\
	$(INC)/grp.h\
	$(INC)/pwd.h\
	$(INC)/stdio.h\
	$(INC)/stdlib.h\
	$(INC)/unistd.h\
	ipcs.c \
	$(FRC)
	$(CC) $(OPTFLAGS) $(CFLAGS) -o ipcs ipcs.c $(LDFLAGS) $(SHLIBS)
	
ipcrm:	$(INC)/sys/types.h\
	$(INC)/sys/ipc.h\
	$(INC)/sys/msg.h\
	$(INC)/sys/sem.h\
	$(INC)/sys/shm.h\
	$(INC)/errno.h\
	$(INC)/stdio.h\
	$(INC)/stdlib.h\
	$(INC)/signal.h\
	ipcrm.c \
	$(FRC)
	$(CC) $(CFLAGS) -o ipcrm ipcrm.c $(LDFLAGS) $(SHLIBS)

listing:
	pr ipc.mk ipcs.c ipcrm.c | $(LIST)

clean:
	rm -f *.o

clobber: clean
	rm -f ipcs ipcrm

lintit:
	lint -x -I$(INC) ipcs.c -lelf
	lint -x -I$(INC) ipcrm.c
