#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)disksetup:disksetup.mk	1.3"

ROOT =
DIR = $(ROOT)/sbin
INC = $(ROOT)/usr/include
BUS = AT386
ARCH=AT386
LDFLAGS =  -s $(ROOTLIBS) 
CFLAGS = -D$(BUS) -I$(INC) -D$(ARCH)
STRIP = strip
OFILES = disksetup.o diskinit.o boot.o
MAINS = disksetup prtvtoc edvtoc partsize

all : diskadd $(MAINS)

install: all
	install -f $(DIR) -m 0544 -u bin -g bin disksetup
	install -f $(DIR) -m 0544 -u bin -g bin partsize
	install -f $(DIR) -m 0544 -u bin -g bin prtvtoc
	install -f $(DIR) -m 0544 -u bin -g bin edvtoc
	install -f $(DIR) -m 0755 -u root -g sys diskadd

disksetup:	$(OFILES) 
	$(CC) -o disksetup  $(OFILES) $(LDFLAGS) -lelf


disksetup.o:	$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/a.out.h \
		$(INC)/sys/hd.h \
		$(INC)/ctype.h \
		$(INC)/sys/types.h \
		$(INC)/sys/vtoc.h \
		$(INC)/sys/alttbl.h \
		$(INC)/sys/fdisk.h \
		$(INC)/malloc.h \
		$(INC)/signal.h 
	$(CC) $(CFLAGS) -c disksetup.c 

diskinit.o:	diskinit.c
	$(CC) $(CFLAGS) -c diskinit.c

boot.o:		boot.c
	$(CC) $(CFLAGS) -c boot.c

partsize: partsize.o
	$(CC) -o partsize partsize.o $(LDFLAGS)
	
prtvtoc: prtvtoc.o
	$(CC) -o prtvtoc prtvtoc.o $(LDFLAGS)
	
edvtoc: edvtoc.o
	$(CC) -o edvtoc edvtoc.o $(LDFLAGS)

clean:
	rm -f *.o

clobber: clean
	rm -f $(MAINS) diskadd

strip: ALL
	$(STRIP) $(MAINS)
