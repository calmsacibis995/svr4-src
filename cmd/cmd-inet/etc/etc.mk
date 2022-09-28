#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cmd-inet:etc/etc.mk	1.9.4.1"

ETC=		$(ROOT)/etc
INITD=		$(ROOT)/etc/init.d
INET=		$(ETC)/inet
STARTINET=	$(ETC)/rc2.d/S69inet
STOPINET1=	$(ETC)/rc1.d/K69inet
STOPINET0=	$(ETC)/rc0.d/K69inet
INSTALL=	install
SYMLINK=	ln -s

FILES=		hosts inetd.conf named.boot networks protocols rc.inet \
		services shells strcf

all:		$(FILES)

install:	all
		@for i in $(INET) $(ETC)/rc2.d $(ETC)/rc1.d $(ETC)/rc0.d;\
		do\
			if [ ! -d $$i ];\
			then\
				mkdir $$i;\
			fi;\
		done;\
		for i in $(FILES);\
		do\
			$(INSTALL) -f $(INET) -m 0444 -u root -g sys $$i;\
			rm -f $(ETC)/$$i;\
			$(SYMLINK) $(INET)/$$i $(ETC)/$$i;\
		done;\
		cd init.d;\
		$(INSTALL) -f $(INITD) -m 0444 -u root -g sys inetinit;\
		rm -f $(STARTINET) $(STOPINET1) $(STOPINET0);\
		ln $(INITD)/inetinit $(STARTINET);\
		ln $(INITD)/inetinit $(STOPINET1);\
		ln $(INITD)/inetinit $(STOPINET0)

clean:

clobber:
