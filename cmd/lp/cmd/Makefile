#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:cmd/Makefile	1.32.3.1"
#
# Makefile for the LP commands
#


TOP	=	..

include ../common.mk


DEBUG	=	-g

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC)

LDFLAGS	=	$(SHLIBS)


#####
#
# $(SRCS) is just for stuff built in the current directory.
#####

SRCS	= \
		cancel.c \
		comb.c \
		lp.c \
		lpfilter.c \
		lpforms.c \
		lpmove.c \
		lpshut.c \
		lpsystem.c \
		lpusers.c \
		dumpolp.c

OBJS	=	$(SRCS:.c=.o)


#####
#
# $(LILCMDS) are those built in the current directory,
# while $(BIGCMDS) are those built in a subdirectory.
# Note: If a subdirectory has further subdivisions, just
# list the first subdirectory here as a ``command'', such
# as "lpsched" (which is actually lpsched, lpexec, and lpNet).
#####

LILCMDS	= \
		accept \
		cancel \
		disable \
		enable \
		lp \
		lpfilter \
		lpforms \
		lpmove \
		lpshut \
		lpsystem \
		lpusers \
		reject \
		dumpolp

BIGCMDS	= \
		bsd \
		lpadmin \
		lpsched \
		lpstat \
		lpdata

CMDS	=	$(BIGCMDS) $(LILCMDS)


#####
#
# $(USRCMDS1) are regular user commands, $(USERCMDS2) are LP
# commands that might be user commands (e.g. disable and enable).
# $(SYSCMDS) are strictly system commands. All of these are $(LILCMDS),
# i.e. commands built in the current directory.
#####

USRCMDS1= \
		cancel \
		lp

USRCMDS2= \
		disable \
		enable

SYSCMDS	= \
		accept \
		lpfilter \
		lpforms \
		lpmove \
		lpshut \
		lpusers \
		lpsystem \
		reject \
		dumpolp


all:		$(CMDS)


#####
#
# Doing "make install" at this level will ensure that the $(LILCMDS)
# are already built, but will simply pass on the make to the
# subdirectories for $(BIGCMDS). In other words, we don't set a
# dependency on $(BIGCMDS) at this level, to avoid redundant work--the
# lower level makefiles take care of things.
#####

install:	localinstall
	for cmd in $(BIGCMDS); \
	do \
		cd ./$$cmd; \
		$(MAKE) SYMLINK="$(SYMLINK)" install; \
		cd ..; \
	done

localinstall:	$(LILCMDS)
	for cmd in $(USRCMDS1); \
	do \
		$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) $$cmd; \
	done
	for cmd in $(USRCMDS2); \
	do \
		$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRBIN) $$cmd; \
	done
	for cmd in $(SYSCMDS); \
	do \
		$(INS) -m $(EMODES) -u $(OWNER) -g $(GROUP) -f $(USRSBIN) $$cmd; \
		$(SYMLINK) $(USRSBIN)/$$cmd $(USRLIB)/$$cmd; \
	done


clean:		localclean
	for cmd in $(BIGCMDS); \
	do \
		cd ./$$cmd; \
		$(MAKE) clean; \
		cd ..; \
	done

localclean:
	$(RM) $(OBJS)

clobber:	localclean
	$(RM) $(LILCMDS)
	for cmd in $(BIGCMDS); \
	do \
		cd ./$$cmd; \
		$(MAKE) clobber; \
		cd ..; \
	done

strip:
	$(STRIP) $(LILCMDS)
	for cmd in $(BIGCMDS); \
	do \
		cd ./$$cmd; \
		$(MAKE) strip; \
		cd ..; \
	done


#####
#
# Need the "nulldependency" because the targets in $(BIGCMDS)
# are directories. Without "nulldependency" make would always
# say the target is up to date.
#####

$(BIGCMDS):	nulldependency
	cd ./$@; $(MAKE)

nulldependency:


accept:		comb.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o accept comb.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)

disable:	accept
	$(CP) accept disable

enable:		disable
	$(LN) disable enable

reject:		accept
	$(LN) accept reject


cancel:		cancel.o \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o cancel cancel.o $(LDFLAGS) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)

lp:		lp.o \
			$(LIBPRT) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lp lp.o $(LDFLAGS) \
			$(LIBPRT) \
			$(LIBREQ) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)

lpfilter:	lpfilter.o \
			$(LIBFLT) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpfilter lpfilter.o $(LDFLAGS) \
			$(LIBFLT) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP) \
				-lgen

lpforms:	lpforms.o \
			$(LIBFRM) \
			$(LIBMSG) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBACC) \
			$(LIBLP)
	$(CC) -o lpforms lpforms.o $(LDFLAGS) \
			$(LIBFRM) \
			$(LIBMSG) \
			$(LIBREQ) \
			$(LIBOAM) \
			$(LIBACC) \
			$(LIBLP)

lpmove:		lpmove.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpmove lpmove.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBLP)

lpshut:		lpshut.o \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpshut lpshut.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)

lpsystem:	lpsystem.o \
			$(LIBSYS) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP)
	$(CC) -o lpsystem lpsystem.o $(LDFLAGS) \
			$(LIBSYS) \
			$(LIBMSG) \
			$(LIBOAM) \
			$(LIBLP) \
				-lnsl

lpusers:	lpusers.o \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBUSR) \
			$(LIBLP)
	$(CC) -o lpusers lpusers.o $(LDFLAGS) \
			$(LIBMSG) \
			$(LIBACC) \
			$(LIBOAM) \
			$(LIBUSR) \
			$(LIBLP)

dumpolp:	dumpolp.o \
			$(LIBLP)
	$(CC) -o dumpolp dumpolp.o $(LDFLAGS) \
			$(LIBLP)


lint:
	$(LINT) $(CFLAGS) comb.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) cancel.c \
			$(LINTREQ) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lp.c \
			$(LINTREQ) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lpfilter.c \
			$(LINTFLT) \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lpforms.c \
			$(LINTFRM) \
			$(LINTMSG) \
			$(LINTACC) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lpmove.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lpshut.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTLP)
	$(LINT) $(CFLAGS) lpusers.c \
			$(LINTMSG) \
			$(LINTOAM) \
			$(LINTUSR) \
			$(LINTLP)
	for cmd in $(BIGCMDS); \
	do \
		cd ./$$cmd; \
		$(MAKE) DEBUG="$(DEBUG)" LINT="$(LINT)" lint; \
		cd ..; \
	done
