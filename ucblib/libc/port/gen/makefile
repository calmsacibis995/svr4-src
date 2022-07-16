#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

#ident	"@(#)ucblibc:port/gen/makefile	1.14.5.1"
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

OBJECTS =  setpriority.o getusershell.o index.o \
	random.o regex.o rindex.o ualarm.o usleep.o \
	bcmp.o bcopy.o bzero.o ndbm.o econvert.o gconvert.o \
	seconvert.o decimal_bin.o ftime.o timezone.o sleep.o \
	nice.o times.o rand.o nlist.o _nlist.o stricmp.o mon.o \
	_psignal.o _swapFLAGS.o fp_class.o getwd.o ieee_func.o \
	ieee_globals.o ieee_handler.o mkstemp.o psignal.o \
	scandir.o setbuffer.o sigfpe.o siglist.o double_decim.o \
	float_decim.o readdir.o


SOURCES =  setpriority.c getusershell.c index.c \
	random.c regex.c rindex.c ualarm.c usleep.c \
	bcmp.c bcopy.c bzero.c ndbm.c econvert.c gconvert.c \
	seconvert.c decimal_bin.c ftime.c timezone.c sleep.c \
	nice.c times.c rand.c nlist.c _nlist.c stricmp.c mon.c \
	_psignal.c _swapFLAGS.c fp_class.c getwd.c ieee_func.c \
	ieee_globals.c ieee_handler.c mkstemp.c psignal.c \
	scandir.c setbuffer.c sigfpe.c siglist.c double_decim.c \
	float_decim.c readdir.c


ALL:		 $(OBJECTS)


double_decim.o: base_conv.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c double_decim.c
 
float_decim.o:  base_conv.h
		$(CC)  $(CFLAGS) -I$(INC1) -c float_decim.c

_psignal.o:	$(INC)/signal.h \
		$(INCSYS)/sys/signal.h
		$(CC) $(CFLAGS) -I$(INC) -c _psignal.c

_swapFLAGS.o:	$(INC1)/math.h \
		$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h
		$(CC) -I$(INC1) $(CFLAGS) -c _swapFLAGS.c

fp_class.o:	$(INC1)/math.h \
		$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h \
		libm.h
		$(CC) -I$(INC1) $(CFLAGS) -c fp_class.c

getwd.o:	$(INCSYS)/sys/param.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/select.h \
		$(INCSYS)/sys/stat.h \
		$(INC1)/sys/dir.h \
		$(INC)/stdio.h \
		$(INCSYS)/sys/fcntl.h \
		$(INC)/unistd.h \
		$(INCSYS)/sys/mman.h \
		$(INCSYS)/sys/mnttab.h \
		$(INCSYS)/sys/errno.h
		$(CC) -I$(INC1) $(CFLAGS) -c getwd.c

ieee_func.o:	$(INC1)/math.h \
		$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h \
		libm.h
		$(CC) -I$(INC1) $(CFLAGS) -c ieee_func.c

ieee_globals.o:	$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h
		$(CC) -I$(INC1) $(CFLAGS) -c ieee_globals.c

ieee_handler.o:	$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h \
		libm.h
		$(CC) -I$(INC1) $(CFLAGS) -c ieee_handler.c

mkstemp.o:	$(INCSYS)/sys/fcntl.h
		$(CC) -I$(INC) $(CFLAGS) -c mkstemp.c

psignal.o:	$(INC)/stdio.h
		$(CC) -I$(INC) $(CFLAGS) -c psignal.c
 
scandir.o:	$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/stat.h \
		$(INC)/dirent.h
		$(CC) -I$(INC) $(CFLAGS) -c scandir.c

setbuffer.o:	$(INC)/stdio.h
		$(CC) -I$(INC) $(CFLAGS) -c setbuffer.c

sigfpe.o:	$(INC)/errno.h \
		$(INC)/signal.h \
		$(INCSYS)/sys/signal.h \
		$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/ucontext.h\
		$(INCSYS)/sys/siginfo.h
		$(CC) -I$(INC1) -I$(INC) $(CFLAGS) -c sigfpe.c

siglist.o:	$(INC)/signal.h \
		$(INCSYS)/sys/signal.h
		$(CC) -I$(INC) $(CFLAGS) -c siglist.c

mon.o:		$(INCSYS)/sys/param.h \
		$(INCSYS)/sys/dir.h \
		mon.h
		$(CC) $(CFLAGS) -I$(INC)  -c mon.c

times.o:	$(INC1)/sys/resource.h \
		$(INCSYS)/sys/time.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c times.c

nlist.o:	$(INC)/a.out.h
		$(CC) $(CFLAGS) -I$(INC) -c nlist.c

_nlist.o:	$(INC)/a.out.h
		$(CC) -Di386 $(CFLAGS) -I$(INC) -c _nlist.c

nice.o:		$(INC1)/sys/resource.h \
		$(INCSYS)/sys/time.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c nice.c

sleep.o:	$(INC1)/signal.h \
		$(INCSYS)/sys/time.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c sleep.c

ftime.o:	$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/time.h
		$(CC) $(CFLAGS) -I$(INC) -c ftime.c

ndbm.o:		$(INC)/stdio.h \
		$(INC)/errno.h \
		$(INC1)/ndbm.h \
		$(INCSYS)/sys/stat.h \
		$(INCSYS)/sys/types.h \
		$(INCSYS)/sys/file.h 
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c ndbm.c

getusershell.o:	 $(INC)/ctype.h \
		 $(INC)/stdio.h \
		 $(INCSYS)/sys/file.h \
		 $(INCSYS)/sys/param.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/stat.h \
		 $(INCSYS)/sys/types.h 
		$(CC) $(CFLAGS) -I$(INC) -c getusershell.c

setpriority.o:	 $(INC)/errno.h \
		 $(INC)/string.h	\
		 $(INCSYS)/sys/errno.h \
		 $(INCSYS)/sys/priocntl.h \
		 $(INCSYS)/sys/procset.h \
		 $(INCSYS)/sys/resource.h \
		 $(INCSYS)/sys/select.h \
		 $(INCSYS)/sys/time.h \
		 $(INCSYS)/sys/tspriocntl.h	\
		 $(INCSYS)/sys/types.h \
		 $(INC)/time.h 
		$(CC) $(CFLAGS) -I$(INC) -c setpriority.c

readdir.o:	 $(INCSYS)/sys/types.h \
		 $(INCSYS1)/sys/dir.h \
		 $(INCSYS)/sys/dirent.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c readdir.c

index.o:	 
		$(CC) $(CFLAGS) -I$(INC) -c index.c

random.o:	 $(INC)/stdio.h 
		$(CC) $(CFLAGS) -I$(INC) -c random.c

regex.o:	 
		$(CC) $(CFLAGS) -I$(INC) -c regex.c

rindex.o:	 
		$(CC) $(CFLAGS) -I$(INC) -c rindex.c

ualarm.o:	 $(INC)/sys/time.h \
		 $(INC)/time.h 
		$(CC) $(CFLAGS) -I$(INC) -c ualarm.c

usleep.o:	 $(INC1)/signal.h	\
		 $(INC)/sys/time.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c usleep.c

econvert.o:	$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c econvert.c

gconvert.o:	$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c gconvert.c

seconvert.o:	$(INC1)/fp.h \
		$(INC1)/sys/ieeefp.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c seconvert.c

decimal_bin.o:	$(INC1)/sunfp.h
		$(CC) $(CFLAGS) -I$(INC1) -I$(INC) -c decimal_bin.c

GLOBALINCS = $(INC)/errno.h \
	$(INC1)/signal.h \
	$(INC)/stdio.h \
	$(INC)/string.h \
	$(INCSYS)/sys/errno.h \
	$(INCSYS)/sys/mau.h \
	$(INCSYS)/sys/param.h \
	$(INCSYS)/sys/pcb.h \
	$(INCSYS)/sys/priocntl.h \
	$(INCSYS)/sys/procset.h \
	$(INCSYS)/sys/psw.h \
	$(INCSYS1)/sys/resource.h \
	$(INCSYS)/sys/select.h \
	$(INCSYS)/sys/siginfo.h \
	$(INCSYS)/sys/signal.h \
	$(INCSYS)/sys/time.h \
	$(INCSYS)/sys/tspriocntl.h \
	$(INCSYS)/sys/types.h \
	$(INCSYS)/sys/ucontext.h \
	$(INC)/time.h \
	$(INC)/ctype.h \
	$(INCSYS)/sys/file.h \
	$(INCSYS)/sys/stat.h \
	$(INC1)/ttyent.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS)
