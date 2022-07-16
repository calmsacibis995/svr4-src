#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucblibc:port/sys/makefile	1.8.5.2"
#
# makefile for ucblibc/port/gen
#
#

CC=cc
CFLAGS= -O
PROF=
NONPROF=
INC=$(ROOT)/usr/include
INC1=$(ROOT)/usr/ucbinclude
INCSYS=$(ROOT)/usr/include
INCSYS1=$(ROOT)/usr/ucbinclude
DEFLIST=
SDEFLIST=

OBJECTS =  gettimeofday.o signal.o \
	wait3.o getrusage.o reboot.o utimes.o \
	gethostname.o killpg.o \
	gethostid.o getdtblsize.o setreid.o \
	getpagesize.o mctl.o


SOURCES =  gettimeofday.c signal.c \
	wait3.c getrusage.c reboot.c utimes.c \
	gethostname.c killpg.c \
	gethostid.c getdtblsize.c setreid.c \
	getpagesize.c mctl.c

ALL:		 $(OBJECTS)

setreid.o:	$(INCSYS)/sys/types.h
		$(CC) $(CFLAGS) -I$(INC) -c setreid.c

getrusage.o:	 $(INCSYS)/sys/errno.h \
		 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/time.h \
		 $(INCSYS)/sys/times.h \
		 $(INCSYS)/sys/types.h \
		 $(INC)/time.h \
		 $(INCSYS1)/sys/resource.h 
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c getrusage.c

gettimeofday.o:	 $(INC)/errno.h \
		 $(INC)/stdio.h \
		 $(INCSYS)/sys/errno.h \
		 $(INCSYS)/sys/evecb.h \
		 $(INCSYS)/sys/hrtcntl.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/time.h \
		 $(INCSYS)/sys/types.h \
		 $(INC)/time.h 
		$(CC) $(CFLAGS) -I$(INC) -c gettimeofday.c

reboot.o:	 $(INCSYS)/sys/errno.h 
		$(CC) $(CFLAGS) -I$(INC) -c reboot.c


signal.o:	 $(INC)/errno.h \
		 $(INC1)/signal.h	\
		 $(INCSYS)/sys/errno.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/siginfo.h \
		 $(INCSYS)/sys/signal.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/ucontext.h 
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c signal.c

utimes.o:	 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/time.h \
		 $(INCSYS)/sys/utime.h \
		 $(INC)/time.h 
		$(CC) $(CFLAGS) -I$(INC) -c utimes.c

wait3.o:	 $(INC)/errno.h \
		 $(INCSYS)/sys/errno.h \
		 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/procset.h \
		 $(INCSYS1)/sys/resource.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/siginfo.h \
		 $(INCSYS)/sys/time.h \
		 $(INCSYS)/sys/times.h \
		 $(INCSYS)/sys/types.h \
		 $(INCSYS)/sys/wait.h \
		 $(INC)/time.h 
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c wait3.c

gethostname.o:	$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/systeminfo.h 
		$(CC) $(CFLAGS) -I$(INC) -c gethostname.c
	
killpg.o:	$(INCSYS)/sys/types.h 
		$(CC) $(CFLAGS) -I$(INC) -c killpg.c

getdtblsize.o:	 $(INCSYS)/sys/resource.h
		$(CC) $(CFLAGS) -I$(INC) -c getdtblsize.c

gethostid.o:	$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/systeminfo.h 
		$(CC) $(CFLAGS) -I$(INC) -c gethostid.c

getpagesize.o:	$(INC)/unistd.h
		$(CC) $(CFLAGS) -I$(INC) -c getpagesize.c

mctl.o:		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/mman.h
		$(CC) $(CFLAGS) -I$(INC) -c mctl.c

GLOBALINCS = $(INC)/errno.h \
	$(INC1)/signal.h \
	$(INC)/stdio.h \
	$(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/evecb.h \
	$(INCSYS)/sys/hrtcntl.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/procset.h \
	$(INCSYS1)/sys/resource.h \
	$(INCSYS)/sys/select.h \
	$(INCSYS)/sys/siginfo.h \
	$(INCSYS)/sys/signal.h \
	$(INCSYS)/sys/time.h \
	$(INCSYS)/sys/times.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/mman.h \
	$(INCSYS)/sys/ucontext.h \
	$(INCSYS)/sys/wait.h \
	$(INC)/time.h \
	$(INC)/ctype.h \
	$(INCSYS)/sys/utime.h \
	$(INCSYS)/sys/systeminfo \
	$(INC)/unistd.h

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS)
