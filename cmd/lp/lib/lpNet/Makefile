#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:lib/lpNet/Makefile	1.10.4.1"

ROOT	=
LPROOT	=	../..
LPLIB	=	$(LPROOT)/lib
LPINC	=	$(LPROOT)/include
BIN	=	$(ROOT)/bin
USRBIN	=	$(ROOT)/usr/bin
INC	=	$(ROOT)/usr/include
INCSYS	=	$(INC)/sys
INCDIR	=	../../include

MAKE	=	make
CC	=	cc
AR	=	ar
STRIP	=	strip
LORDER	=	lorder
TSORT	=	tsort
LINT	=	lint
RPCGEN	=	rpcgen
ENV	=	

LIB	=	liblpNet.a
TARGETS	=	$(LIB)

LINTSRC	=	llib-llpNet.c
LINTLIB	=	$(LINTSRC:.c=.ln)

SRCS	=	networkMgmt.c	\
		_networkMgmt.c	\
		xdrMsgs.c	\
		_xdrMsgs.c	\
		errorMgmt.c	\
		logMgmt.c	\
		mpipes.c	\
		lists.c		\
		memdup.c

XDRSRCS	=	xdrMsgs.x	\
		_xdrMsgs.x

OBJS	=	$(SRCS:.c=.o)

DEBUG	=	-DDEBUG
CFLAGS	=	-I$(INCDIR) -I$(INC) $(DEBUG)
ARFLAGS	=	rv

.PRECIOUS:	$(SRCS) $(XDRSRCS) $(TARGETS)

all:		$(LIB)
		@for t in $(TARGETS);\
		do\
			echo  "$$t is-up-to-date.";\
		done

install:	all

clean:
		rm -f *.o

clobber:	clean
		rm -f $(TARGETS) $(LINTLIB)

lint:
		$(LINT) -I$(INCDIR) -I$(INC) $(SRCS)

lintsrc:	$(LINTSRC)

$(LINTSRC):	
	if [ -n "$(FUNCDCL)" ]; then $(FUNCDCL) $(SRCS) >$(LINTSRC); fi

lintlib:	$(LINTLIB)

$(LINTLIB):	$(LINTSRC)
	$(LINT) -o lpNet -x -v $(CFLAGS) $(LINTFLAGS) $(LINTSRC)

product:
		@for t in $(TARGETS);\
		do\
			echo  $$t;\
		done

rpcgen:		clean_rpcgen
		@if [ ! -x $(RPCGEN) -a ! -x /usr/bin/$(RPCGEN) ];\
		then\
			echo "ERROR:\007";\
			echo "\t\`\`$(RPCGEN)'' does not exist in this \c";\
			echo "environment.";\
			exit 1;\
		fi
		touch _xdrMsgs.x xdrMsgs.x
		echo "#""ident\t\"@(#)lp:lib/lpNet/Makefile	1.10.4.1\"" >$(INCDIR)/_xdrMsgs.h
		$(RPCGEN) -DRPCGEN -h _xdrMsgs.x >>$(INCDIR)/_xdrMsgs.h
		echo "#""ident\t\"@(#)lp:lib/lpNet/Makefile	1.10.4.1\"" >_xdrMsgs.c
		$(RPCGEN) -DRPCGEN -c _xdrMsgs.x >>_xdrMsgs.c
		echo "#""ident\t\"@(#)lp:lib/lpNet/Makefile	1.10.4.1\"" >$(INCDIR)/xdrMsgs.h
		$(RPCGEN) -DRPCGEN -h xdrMsgs.x >>$(INCDIR)/xdrMsgs.h
		echo "#""ident\t\"@(#)lp:lib/lpNet/Makefile	1.10.4.1\"" >xdrMsgs.c
		$(RPCGEN) -DRPCGEN -c xdrMsgs.x >>xdrMsgs.c

clean_rpcgen:
		rm -f _xdrMsgs.c xdrMsgs.c
		rm -f $(INCDIR)/_xdrMsgs.h $(INCDIR)/xdrMsgs.h

$(LIB):		$(XDRSRCS) $(OBJS)
		$(AR)  $(ARFLAGS)  $(LIB)  `$(LORDER) *.o | $(TSORT)`


xdrMsgs.x:

_xdrMsgs.x:	

networkMgmt.o:	$(INCDIR)/networkMgmt.h	\
		$(INCDIR)/errorMgmt.h  $(INCDIR)/lists.h	\
		$(INCDIR)/memdup.h

_networkMgmt.o:	$(INCDIR)/_networkMgmt.h	\
		$(INCDIR)/errorMgmt.h

xdrMsgs.o:	$(INCDIR)/xdrMsgs.h

_xdrMsgs.o:	$(INCDIR)/_xdrMsgs.h

logMgmt.o:	$(INCDIR)/logMgmt.h

errorMgmt.o:	$(INCDIR)/errorMgmt.h

mpipes.o:	$(INCDIR)/mpipes.h

lists.o:	$(INCDIR)/lists.h

memdup.o:	$(INCDIR)/memdup.h

$(INCDIR)/_networkMgmt.h:\
		$(INCDIR)/_xdrMsgs.h	$(INCDIR)/boolean.h
		touch $@

$(INCDIR)/networkMgmt.h:\
		$(INCDIR)/xdrMsgs.h	$(INCDIR)/_networkMgmt.h \
		$(INCDIR)/lists.h	$(INCDIR)/boolean.h
		touch $@

$(INCDIR)/xdrMsgs.h:	xdrMsgs.x 
		@echo "\tWARNING:\007"
		@echo "\t\`\`xdrMsgs.x'' \c"
		@echo "newer than \`\`$(INCDIR)/xdrMsgs.h''."
		@echo "\tUse \`\`make rpcgen all''."

$(INCDIR)/_xdrMsgs.h:	_xdrMsgs.x
		@echo "\tWARNING:\007"
		@echo "\t\`\`_xdrMsgs.x'' \c"
		@echo "newer than \`\`$(INCDIR)/_xdrMsgs.h''."
		@echo "\tUse \`\`make rpcgen all''."

$(INCDIR)/logMgmt.h:

$(INCDIR)/errorMgmt.h:

$(INCDIR)/mpipes.h:

$(INCDIR)/lists.h:

$(INCDIR)/memdup.h:

