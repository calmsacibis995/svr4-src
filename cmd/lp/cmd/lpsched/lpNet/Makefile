#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:cmd/lpsched/lpNet/Makefile	1.13.3.1"

LPROOT	=	../../..
LPNETROOT=	.
LPLIB	=	$(LPROOT)/lib
LPINC	=	$(LPROOT)/include
USRBIN	=	$(ROOT)/usr/bin
INC	=	$(ROOT)/usr/include
INCSYS	=	$(INC)/sys

USRLIBLP=	$(ROOT)/usr/lib/lp
ETCLP	=	$(ROOT)/etc/lp

MAKE	=	make
CC	=	cc
AS	=	as
AR	=	ar
STRIP	=	strip
LINT	=	lint
ENV	=

INSTALL	=	install
OWNER	=	lp
GROUP	=	tty
MODES	=	0555

NETLIBS =	parent/parent.a	\
		svChild/svChild.a	\
		bsdChild/bsdChild.a

LPNETLIB=	$(LPLIB)/lpNet/liblpNet.a
LIBBSD	=	$(LPLIB)/bsd/liblpbsd.a

LPLIBS	=	$(LPNETLIB)			\
		$(LIBBSD)			\
		$(LPLIB)/oam/liblpoam.a		\
		$(LPLIB)/requests/liblpreq.a	\
		$(LPLIB)/msgs/liblpmsg.a	\
		$(LPLIB)/secure/liblpsec.a	\
		$(LPLIB)/printers/liblpprt.a	\
		$(LPLIB)/systems/liblpsys.a	\
		$(LPLIB)/lp/liblp.a
SHLIBS	=
DEBUG	=	-DDEBUG
LDFLAGS	=	$(SHLIBS)
GLIB	=
SYSLIBS	=	-lnls -lnsl -lgen -lcurses $(GLIB)
LIBS	=	$(NETLIBS) $(LPLIBS) $(SYSLIBS)

SRCDIRS	=	parent svChild bsdChild
TARGETS	=	lpNet

MAKEARGS=	DEBUG="$(DEBUG)"
MAKECMD	=	$(MAKE)  $(MAKEARGS)

.PRECIOUS:	$(TARGETS) $(NETLIBS)


all:		$(TARGETS)
		@for t in $(TARGETS);			\
		do					\
			echo "$${t} is-up-to-date.";	\
		done

install:	all
		$(INSTALL) -m $(MODES) -u $(OWNER) -g $(GROUP) \
			-f $(USRLIBLP) lpNet
		$(INSTALL) -m 644 -u $(OWNER) -g $(GROUP) \
			-f $(ETCLP) Systems

strip:
		$(STRIP) lpNet

clean:
		@for d in $(SRCDIRS); \
		do \
			cd $${d}; \
			$(MAKECMD) $@; \
			cd ..; \
		done

clobber:
		@for d in $(SRCDIRS); \
		do \
			cd $${d}; \
			$(MAKECMD) $@; \
			cd ..; \
		done
		rm -f $(TARGETS)

product:
		@for t in $(TARGETS);	\
		do			\
			echo "$${t}";	\
		done

lint:
		@for d in $(SRCDIRS); \
		do \
			cd $${d}; \
			$(MAKECMD) $@; \
			cd ..; \
		done

lpNet:		$(NETLIBS)
		$(CC) $(LDFLAGS) $(LIBS) -o lpNet

#parent/parent.a:	FRC
parent/parent.a:
	cd parent; $(MAKECMD) all

#svChild/svChild.a:	FRC
svChild/svChild.a:
	cd svChild; $(MAKECMD) all

#bsdChild/bsdChild.a:	FRC
bsdChild/bsdChild.a:
	cd bsdChild; $(MAKECMD) all

FRC:
