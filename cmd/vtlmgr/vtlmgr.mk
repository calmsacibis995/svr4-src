#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vtlmgr:vtlmgr.mk	1.3"

ROOT =
DIR = $(ROOT)/usr/bin
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)
CFLAGS = -O -I$(INC)

# to install when not privileged
# set $(CH) in the environment to #

MAINS = vtlmgr newvt vtgetty
OBJECTS =  vtlmgr.o newvt.o vtgetty.o
SOURCES =  vtlmgr.c newvt.c vtgetty.c

all: $(MAINS)

install: all
	install -f $(ROOT)/usr/bin -m 2555 -u bin -g tty vtlmgr
	install -f $(ROOT)/usr/bin -m 555 -u bin -g bin newvt
	install -f $(ROOT)/sbin -m 544 -u root -g bin vtgetty

vtlmgr:	vtlmgr.o
	$(CC) $(CFLAGS) -o vtlmgr vtlmgr.o $(LDFLAGS)

vtlmgr.o: $(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(INC)/sys/at_ansi.h \
	$(INC)/sys/kd.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vt.h

newvt: newvt.o
	$(CC) $(CFLAGS) -o newvt newvt.o $(LDFLAGS) 

newvt.o: $(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(INC)/ctype.h \
	$(INC)/sys/at_ansi.h \
	$(INC)/sys/kd.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vt.h

vtgetty: vtgetty.o
	$(CC) $(CFLAGS) -o vtgetty vtgetty.o $(LDFLAGS)

vtgetty.o: $(INC)/fcntl.h \
	$(INC)/signal.h \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(INC)/sys/at_ansi.h \
	$(INC)/sys/kd.h \
	$(INC)/sys/termio.h \
	$(INC)/sys/types.h \
	$(INC)/sys/vt.h

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(MAINS)
