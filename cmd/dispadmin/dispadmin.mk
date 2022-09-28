#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dispadmin:dispadmin.mk	1.5.1.1"
INC = $(ROOT)/usr/include
GENDIR = $(ROOT)/usr/sbin
CLASSDIR = $(ROOT)/usr/lib/class
LDFLAGS = -lgen -s
CFLAGS = -O -I$(INC)
INS = install

all: dispadmin classes

dispadmin: dispadmin.c subr.o\
	$(INC)/stdio.h\
	$(INC)/string.h\
	$(INC)/unistd.h\
	$(INC)/sys/types.h\
	$(INC)/sys/priocntl.h
	$(CC) $(CFLAGS) dispadmin.c subr.o -o dispadmin $(LDFLAGS) $(SHLIBS)

classes: RTdispadmin TSdispadmin

RTdispadmin: rtdispadmin.c subr.o\
	$(INC)/stdio.h\
	$(INC)/string.h\
	$(INC)/sys/types.h\
	$(INC)/sys/priocntl.h\
	$(INC)/sys/rtpriocntl.h\
	$(INC)/sys/param.h\
	$(INC)/sys/evecb.h\
	$(INC)/sys/hrtcntl.h\
	$(INC)/sys/rt.h
	$(CC) $(CFLAGS) rtdispadmin.c subr.o -o RTdispadmin $(LDFLAGS) $(SHLIBS)

TSdispadmin: tsdispadmin.c subr.o\
	$(INC)/stdio.h\
	$(INC)/string.h\
	$(INC)/sys/types.h\
	$(INC)/sys/priocntl.h\
	$(INC)/sys/tspriocntl.h\
	$(INC)/sys/param.h\
	$(INC)/sys/evecb.h\
	$(INC)/sys/hrtcntl.h\
	$(INC)/errno.h
	$(CC) $(CFLAGS) tsdispadmin.c subr.o -o TSdispadmin $(LDFLAGS) $(SHLIBS)

subr.o: subr.c\
	$(INC)/stdio.h\
	$(INC)/sys/types.h\
	$(INC)/sys/evecb.h\
	$(INC)/sys/hrtcntl.h
	$(CC) $(CFLAGS) -c subr.c


install: all
	$(INS) -f $(GENDIR) -u bin -g bin -m 555 dispadmin
	@if [ ! -d $(CLASSDIR)/RT ]; \
		then \
		mkdir $(CLASSDIR)/RT; \
		fi;
	@if [ ! -d $(CLASSDIR)/TS ]; \
		then \
		mkdir $(CLASSDIR)/TS; \
		fi;
	$(INS) -f $(CLASSDIR)/RT -u bin -g bin -m 555 RTdispadmin
	$(INS) -f $(CLASSDIR)/TS -u bin -g bin -m 555 TSdispadmin

clean:
	rm -f *.o

clobber: clean
	rm -f dispadmin RTdispadmin TSdispadmin
