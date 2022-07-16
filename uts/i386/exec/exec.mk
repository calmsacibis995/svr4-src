#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-exe:exec.mk	1.3.1.1"

STRIP = strip
INCRT = ../..
CC = cc

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS)
DEFLIST =
FRC =

all:	exectypes

exectypes:
	@for i in *;\
	do\
		if [ -d $$i -a -f $$i/$$i.mk ];then\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd  $$i;\
			echo "====== $(MAKE) -f $$i.mk \"MAKE=$(MAKE)\" \"AS=$(AS)\" \"CC=$(CC)\" \"LD=$(LD)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"MORECPP=$(MORECPP)\" \"DASHO=$(DASHO)\"";\
		$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "AS=$(AS)" "CC=$(CC)" "LD=$(LD)" "FRC=$(FRC)" "INCRT=$(INCRT)" "MORECPP=$(MORECPP)" "DASHO=$(DASHO)"; \
		cd .. ;; \
		esac;\
		fi;\
	done

clean:
	-rm -f *.o
	@for i in *; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ]; then \
			cd $$i; \
			$(MAKE) -f $$i.mk clean; \
			cd ..; \
		fi; \
	done

clobber:	clean
	@for i in *; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ]; then \
			cd $$i; \
			$(MAKE) -f $$i.mk clobber; \
			cd ..; \
		fi; \
	done
