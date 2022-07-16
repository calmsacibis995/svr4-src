#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kdb:kdb.mk	1.3.2.2"

BUS = AT386
ARCH = AT386
MORECPP = -D$(BUS) -D$(ARCH)
DEFLIST =
FRC =

INCRT = ..
CC = cc
DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
CONF = $(ROOT)/etc/conf

all:	subdirs

subdirs:
	@for d in *; \
	do \
		if [ -d $$d -a -f $$d/$$d.mk ]; then \
			(cd $$d; \
			echo "====== $(MAKE) -f $$d.mk all"; \
			$(MAKE) -f $$d.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" \
				"LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" \
				"CONF=$(CONF)" all ) ; \
		fi; \
	done

clean:
	@for d in *; \
	do \
		if [ -d $$d -a -f $$d/$$d.mk ]; then \
			(cd $$d; \
			echo "====== $(MAKE) -f $$d.mk clean"; \
			$(MAKE) -f $$d.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" \
				"LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" \
				"CONF=$(CONF)" clean ) ; \
		fi; \
	done

clobber:
	@for d in *; \
	do \
		if [ -d $$d -a -f $$d/$$d.mk ]; then \
			(cd $$d; \
			echo "====== $(MAKE) -f $$d.mk clobber"; \
			$(MAKE) -f $$d.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" \
				"LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" \
				"CONF=$(CONF)" clobber ) ; \
		fi; \
	done
