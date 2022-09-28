#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)priocntl:priocntl.mk	1.4.3.1"
INC = $(ROOT)/usr/include
GENDIR = $(ROOT)/usr/bin
CLASSDIR = $(ROOT)/usr/lib/class
LDFLAGS = -lgen -s $(SHLIBS)
CFLAGS = -O -I$(INC)
INS = install

all: priocntl classes

priocntl: priocntl.c subr.o\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/dirent.h\
	$(INC)/fcntl.h\
	$(INC)/sys/procset.h\
	$(INC)/sys/priocntl.h\
	$(INC)/sys/procfs.h\
	$(INC)/macros.h\
	$(INC)/errno.h\
	priocntl.h
	$(CC) $(CFLAGS) priocntl.c subr.o -o priocntl $(LDFLAGS)

classes: RTpriocntl TSpriocntl

RTpriocntl: rtpriocntl.c subr.o\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/sys/procset.h\
	$(INC)/sys/priocntl.h\
	$(INC)/sys/rtpriocntl.h\
	$(INC)/sys/param.h\
	$(INC)/sys/evecb.h\
	$(INC)/sys/hrtcntl.h\
	$(INC)/limits.h\
	$(INC)/errno.h\
	priocntl.h
	$(CC) $(CFLAGS) rtpriocntl.c subr.o -o RTpriocntl $(LDFLAGS)

TSpriocntl: tspriocntl.c subr.o\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/sys/procset.h\
	$(INC)/sys/priocntl.h\
	$(INC)/sys/tspriocntl.h\
	$(INC)/errno.h\
	priocntl.h
	$(CC) $(CFLAGS) tspriocntl.c subr.o -o TSpriocntl $(LDFLAGS)

subr.o: subr.c\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/sys/procset.h\
	$(INC)/sys/priocntl.h\
	priocntl.h
	$(CC) $(CFLAGS) -c subr.c


install: all
	$(INS) -f $(GENDIR) -u root -g root -m 4555 priocntl
	@if [ ! -d $(CLASSDIR)/RT ]; \
		then \
		mkdir $(CLASSDIR)/RT; \
		fi;
	@if [ ! -d $(CLASSDIR)/TS ]; \
		then \
		mkdir $(CLASSDIR)/TS; \
		fi;
	$(INS) -f $(CLASSDIR)/RT -u bin -g bin -m 555 RTpriocntl
	$(INS) -f $(CLASSDIR)/TS -u bin -g bin -m 555 TSpriocntl

clean:
	rm -f *.o

clobber: clean
	rm -f priocntl RTpriocntl TSpriocntl
