#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rmount:rmount.mk	1.2.13.1"


ROOT = 
TESTDIR = .
INSDIR = $(ROOT)/etc/rfs
INC = $(ROOT)/usr/include
INS = install
CFLAGS=$(DEBUG) $(PROFILE) -s -I$(INC)
LDFLAGS=-lns $(ROOTLIBS)
CC = cc
PROFILE=
DEBUG=
EXECS=rmount rmnttry rumount
SOURCE=rmount.c rmnttry.c rumount.c fqn.c mntlock.c rd_rmnttab.c wr_rmnttab.c
RMOBJECTS = rmount.o mntlock.o rd_rmnttab.o wr_rmnttab.o fqn.o
RUOBJECTS = rumount.o mntlock.o rd_rmnttab.o wr_rmnttab.o fqn.o
RTOBJECTS = rmnttry.o mntlock.o rd_rmnttab.o wr_rmnttab.o fqn.o
FRC =

all:	$(EXECS)
rmount: $(RMOBJECTS)
	$(CC) $(CFLAGS) $(RMOBJECTS) -o $(TESTDIR)/rmount $(LDFLAGS)
rmnttry: $(RTOBJECTS)
	$(CC) $(CFLAGS) $(RTOBJECTS) -o $(TESTDIR)/rmnttry $(LDFLAGS)
rumount: $(RUOBJECTS)
	$(CC) $(CFLAGS) $(RUOBJECTS) -o $(TESTDIR)/rumount $(LDFLAGS)

fqn.o: $(INC)/sys/types.h $(INC)/sys/rf_sys.h $(INC)/nserve.h $(INC)/string.h
	$(CC) -I$(INC) -c $(CFLAGS) fqn.c

mntlock.o: $(INC)/sys/types.h $(INC)/sys/errno.h $(INC)/sys/stat.h \
	$(INC)/unistd.h $(INC)/fcntl.h $(INC)/stdio.h
	$(CC) -I$(INC) -c $(CFLAGS) mntlock.c

rd_rmnttab.o: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h
	$(CC) -I$(INC) -c $(CFLAGS) rd_rmnttab.c

rmnttry.c: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h
	$(CC) -I$(INC) -c $(CFLAGS) rmnttry.c

rmount.o: $(INC)/sys/types.h  $(INC)/mnttab.h $(INC)/stdio.h $(INC)/nserve.h
	$(CC) -I$(INC) -c $(CFLAGS) rmount.c

rumount.o: $(INC)/sys/types.h  $(INC)/mnttab.h $(INC)/stdio.h $(INC)/nserve.h
	$(CC) -I$(INC) -c $(CFLAGS) rumount.c

wr_rmnttab.o: $(INC)/sys/types.h $(INC)/sys/stat.h $(INC)/mnttab.h \
	$(INC)/stdio.h $(INC)/fcntl.h $(INC)/unistd.h
	$(CC) -I$(INC) -c $(CFLAGS) wr_rmnttab.c 

install: all
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		mkdir $(INSDIR) ; \
	fi ;
	-rm -f $(ROOT)/usr/nserve/rmount
	-rm -f $(ROOT)/usr/nserve/rmnttry
	-rm -f $(ROOT)/usr/nserve/rumount
	$(INS) -f $(INSDIR) -m 555 -u root -g sys rmount
	$(INS) -f $(INSDIR) -m 550 -u root -g sys rmnttry
	$(INS) -f $(INSDIR) -m 555 -u root -g sys rumount
	-@if [ ! -d "$(ROOT)/usr/nserve" ] ; \
	then \
		mkdir $(ROOT)/usr/nserve ; \
	fi ;
	-$(SYMLINK) /etc/rfs/rmount $(ROOT)/usr/nserve/rmount
	-$(SYMLINK) /etc/rfs/rmnttry $(ROOT)/usr/nserve/rmnttry
	-$(SYMLINK) /etc/rfs/rumount $(ROOT)/usr/nserve/rumount

uninstall:
	(cd $(INSDIR); rm -f $(EXECS))

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(EXECS)
FRC: 

