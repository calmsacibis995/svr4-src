#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)devintf:devices/add/add.mk	1.8.3.1"

DIR		= $(ROOT)/bin
OAMBASE		= /usr/sadm/sysadm
BINDIR		= $(ROOT)$(OAMBASE)/bin
INC		= $(ROOT)/usr/include
DESTDIR		= $(ROOT)$(OAMBASE)/menu/devices/devices/add
HELPSRCDIR 	= .
INS	 	= install
STRIP 		= strip

SHFILES		=
FMTFILES	=
DISPFILES	= Form.add Text.c_add
HELPFILES	= Help

all: $(SHFILES) $(HELPFILES) $(FMTFILES) $(DISPFILES)

# $(SHFILES):

# $(FMTFILES):

$(HELPFILES): 

$(DISPFILES):

clean:

clobber: clean

size:

strip:

install: $(DESTDIR) all
	for i in $(DISPFILES) ;\
	do \
		$(INS) -m 640 -g bin -u bin -f $(DESTDIR) $$i ;\
	done
	for i in $(HELPFILES) ;\
	do \
		$(INS) -m 640 -g bin -u bin -f $(DESTDIR) $(HELPSRCDIR)/$$i ;\
	done
#	for i in $(FMTFILES) ;\
#	do \
#		$(INS) -m 640 -g bin -u bin -f $(DESTDIR) $$i ;\
#	done
#	for i in $(SHFILES) ;\
#	do \
#		$(INS) -m 750 -g bin -u bin -f $(DESTDIR) $$i ;\
#	done

$(DESTDIR):
	builddir() \
	{ \
		if [ ! -d $$1 ]; \
		then \
		    builddir `dirname $$1`; \
		    mkdir $$1; \
		fi \
	}; \
	builddir $(DESTDIR)
