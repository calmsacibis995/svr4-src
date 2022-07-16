#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/examples/makefile	1.3"

SOURCE = rec.c send.c common.c
HEADER = ft.h
MISC = makefile README
OBJECTS = rec.o send.o common.o
EXECUTABLES = rec send
TRACE = temp.c a.out traceout temp.o
CC = cc
CCOPTS = -g -c # use -g option to allow use of sdb
LIB = -lmb2

everything: send rec

send:	send.o common.o
	$(CC) -osend send.o common.o $(LIB)

rec:	rec.o common.o
	$(CC) -orec rec.o common.o $(LIB)

common.o:	common.c $(HEADER)
	$(CC) $(CCOPTS) common.c

send.o:	send.c $(HEADER)
	$(CC) $(CCOPTS) send.c

rec.o:	rec.c $(HEADER)
	$(CC) $(CCOPTS) rec.c


# remove trace files and .o files
clean:
	rm -f $(OBJECTS) $(TRACE)

# remove all non-source files (including executables)
clobber:
	rm -f $(OBJECTS) $(EXECUTABLES) $(TRACE)

# Put all the source on a floppy
cpio:
	ls $(SOURCE) $(HEADER) $(MISC) |cpio -ocBv >/dev/rdsk/f0d9dt

# make a rec with ctrace.  run it as ./a.out >ctraceout
# clean and clobber will remove all trace files
rectrace:
	$(MAKE)
	ctrace rec.c >temp.c
	cc temp.c common.o $(LIB)

# make a send with ctrace.  run it as ./a.out [options] >ctraceout
# clean and clobber will remove all trace files
sendtrace:
	$(MAKE)
	ctrace send.c >temp.c
	cc temp.c common.o $(LIB)
