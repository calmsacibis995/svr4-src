#ident	"@(#)Makefile	1.2	91/09/22	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:cmd/lpsched/lpsched/Makefile	1.8.3.1"
#
# Makefile for the lpsched command
#


TOP	=	../../..

include ../../../common.mk


#
# Possible uses of TRACE_MALLOC and MALLOC_INC:
#
#	make TRACE_MALLOC=-DTRACE_MALLOC
#	make MALLOC_INC=-I/usr/foo/include
#	make TRACE_MALLOC=-DMDL MALLOC_INC=-I/usr/foo/include
#
# Define MALLOC_3X if you are using malloc(3X). Link it in
# by defining LDMALLOC=-lmalloc.
#
# Define LOST_LOCK if the mysterious problem of losing the lock
# on the SCHEDLOCK file occurs. It's been seen to happen when
# the system is under heavy load.
#
# Define CHECK_CHILDREN to cause the scheduler to look for children
# which have terminated without sending S_CHILD_DONE.  This may
# occur if the intermediate child core dumps or is killed by someone.
# Update: Starting with SVR4.0 we now use a better method of learning
# about finished child processes, so this option should no longer be
# needed.
#

#CHECK_CHILDREN	= -DCHECK_CHILDREN
LOST_LOCK	= -DLOST_LOCK
#MALLOC_3X	= -DMALLOC_3X -DDEF_MXFAST=32 -DDEF_NLBLKS=32 -DDEF_GRAIN=4
#LDMALLOC	= -lmalloc
#TRACE_MALLOC	= -DTRACE_MALLOC
#LDDEBUG	= malloc/mymalloc.c


DEBUG	=

#
# Doing -DDEBUG allows some nice log files to be generated
# with the -d option, so don't let the top level makefile
# override this feature.
MYDEBUG	=	$(DEBUG) -DDEBUG $(TRACE_MALLOC) $(MALLOC_INC)

CFLAGS	=	-I. -I$(LPINC) -I$(INC) $(MYDEBUG) \
			$(CHECK_CHILDREN) \
			$(LOST_LOCK) \
			$(MALLOC_3X)

LDFLAGS	=	$(SHLIBS)


HDRS	= \
		nodes.h \
		dispatch.h \
		validate.h \
		lpsched.h

SRCS	= \
		alerts.c \
		cancel.c \
		checkchild.c \
		daisyforms.c \
		disena.c \
		disp1.c \
		disp2.c \
		disp3.c \
		disp4.c \
		disp5.c \
		disptab.c \
		dowait.c \
		exec.c \
		faults.c \
		files.c \
		flt.c \
		fncs.c \
		getkey.c \
		getpwent.c \
		getstatus.c \
		init.c \
		log.c \
		lpfsck.c \
		lpsched.c \
		msgs.c \
		notify.c \
		pickfilter.c \
		ports.c \
		putjob.c \
		rexec.c \
		requeue.c \
		rstatus.c \
		schedule.c \
		status.c \
		terminate.c \
		validate.c


OBJS	=	$(SRCS:.c=.o)

LIBS	= \
		$(LIBMSG) \
		$(LIBFRM) \
		$(LIBREQ) \
		$(LIBPRT) \
		$(LIBCLS) \
		$(LIBACC) \
		$(LIBFLT) \
		$(LIBUSR) \
		$(LIBOAM) \
		$(LIBLP) \
		$(LIBSEC) \
		$(LIBSYS)

SYSLIBS	=	-lcurses -lgen -lcurses -lnsl

LINTS	= \
		$(LINTMSG) \
		$(LINTFRM) \
		$(LINTREQ) \
		$(LINTPRT) \
		$(LINTCLS) \
		$(LINTACC) \
		$(LINTFLT) \
		$(LINTUSR) \
		$(LINTOAM) \
		$(LINTLP) \
		$(LINTSEC) \
		$(LINTSYS)


CMD	=	lpsched


all:		$(CMD)

install:	all
	$(INS) -m $(SMODES) -u $(SUPER) -g $(GROUP) -f $(USRLIBLP) $(CMD)
	$(SYMLINK) $(USRLIBLP)/$(CMD) $(USRLIB)/$(CMD)

clean:
	$(RM) $(OBJS)

clobber:	clean
	$(RM) $(CMD)

strip:
	$(STRIP) $(CMD)

$(CMD):		$(OBJS) $(LIBS)
	$(CC) -o $(CMD) $(OBJS) $(LDFLAGS) $(LIBS) \
					$(LDMALLOC) \
					$(LDDEBUG) \
						$(SYSLIBS)

$(OBJS):	$(HDRS)

lint:
	$(LINT) -x $(CFLAGS) $(SRCS) $(LINTS)

MODULE_OBJS	=	$(MODULE_SRC:.c=.o)

module:		$(MODULE_SRC:.c=.o) $(LIBS)
	$(CC) -o $(MODULE) $(MODULE_SRC:.c=.o) $(LDFLAGS) $(LIBS) \
					$(LDMALLOC) \
					$(LDDEBUG) \
						$(SYSLIBS)
