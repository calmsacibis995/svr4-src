#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)make:make.mk	1.11.5.1"
########
#
#	MAKE/BUILD MAKEFILE
#
########
#
#	Standard Macros
#
########
AUX_CLEAN = 
CC = cc
CC_CMD = $(CC) -c $(PPDEFS) $(CFLAGS) $(INC_LIST) $(LINK_MODE)
CFLAGS = -O
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
CMDBASE=..
INS=$(CMDBASE)/install/install.sh
INSDIR=$(CCSBIN)
STRIP=strip
DEFS = 
LDFLAGS = -O
LINK_MODE=
MAKE.LO = make.lo
MAKE.ROOT = $(ROOT)/make.root
MKGEN = mkgen
MV = mv
PPDEFS = $(DEFS)
ROOT =
SGS = $(SGSX)
SGSBASE=../sgs
SGSX = 
YACC = yacc

INC_LIST	=\
	-I. -I$(SGSBASE)/inc/common


CLEAN =\
	doname.o\
	dosys.o\
	dyndep.o\
	files.o\
	gram.c\
	gram.o\
	main.o\
	misc.o\
	prtmem.o\
	rules.o

##########
#
#	make.root
#
##########

INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include

########
#
#	make.lo
#
########

TITLE = MAKE/BUILD MAKEFILE

PRODUCTS = make 

HELPLIB = $(CCSLIB)/help
TESTDIR = .
YACC_CMD = $(YACC)
MKLIB = -lld 

OBJECTS = main.o doname.o misc.o files.o rules.o dosys.o \
	gram.o dyndep.o prtmem.o 

SOURCES = main.c doname.c misc.c files.c rules.c dosys.c \
	gram.y dyndep.c prtmem.c make.mk

all:	make 
	@echo "make is up to date"

make:	$(OBJECTS)
	if pdp11 ; then \
		$(CC) $(LDFLAGS) $(OBJECTS) -o $(TESTDIR)/make ; \
	else \
		$(CC) $(LINK_MODE) $(LDFLAGS) $(OBJECTS) $(MKLIB) -o $(TESTDIR)/make ; \
	fi


install:	all
	cp make	make.bak
	$(STRIP) make
	/bin/sh	$(INS) -f $(INSDIR) make
	mv make.bak make
	-mkdir $(HELPLIB)
	-chmod 775 $(HELPLIB)
	cp bu $(HELPLIB)/bu
	-cd $(HELPLIB); chmod 664 bu
	-@cd $(HELPLIB); chgrp bin bu .


########
#
#	All dependencies and rules not explicitly stated
#	(including header and nested header dependencies)
#
########

doname.o:	defs
doname.o:	doname.c
doname.o:	$(INC)/errno.h
doname.o:	$(INC)/stdio.h # nested include from defs
doname.o:	$(INC)/sys/errno.h # nested include from errno.h
doname.o:	$(INC)/sys/stat.h
doname.o:	$(INC)/sys/types.h
doname.o:	$(INC)/time.h
	$(CC_CMD) doname.c

dosys.o:	defs
dosys.o:	dosys.c
dosys.o:	$(INC)/stdio.h # nested include from defs
dosys.o:	$(INC)/sys/types.h
	if [ X$(NATIVE) = Xno ] ;\
	then \
		$(CC_CMD) -DMAKE_SHELL="\"/bin/sh\"" dosys.c ;\
	else \
		$(CC_CMD) dosys.c ;\
	fi


dyndep.o:	defs
dyndep.o:	dyndep.c
dyndep.o:	$(INC)/stdio.h # nested include from defs
	$(CC_CMD) dyndep.c

files.o:	$(INC)/ar.h
files.o:	defs
files.o:	files.c
files.o:	$(INC)/pwd.h
files.o:	$(INC)/stdio.h # nested include from defs
files.o:	$(INC)/sys/dir.h
files.o:	$(INC)/sys/stat.h
files.o:	$(INC)/sys/types.h
	$(CC_CMD) files.c

gram.c:	gram.y
gram.c: defs
	$(YACC_CMD) gram.y
	$(MV) y.tab.c gram.c

gram.o:	$(INC)/ctype.h
gram.o:	gram.c
gram.o:	$(INC)/stdio.h # nested include from defs
	$(CC_CMD) gram.c

main.o:	defs
main.o:	main.c
main.o:	$(INC)/signal.h
main.o:	$(INC)/stdio.h # nested include from defs
main.o:	$(INC)/sys/signal.h # nested include from signal.h
main.o:	$(INC)/time.h
	$(CC_CMD) main.c

misc.o:	$(INC)/ctype.h
misc.o:	defs
misc.o:	$(INC)/errno.h
misc.o:	misc.c
misc.o:	$(INC)/signal.h
misc.o:	$(INC)/stdio.h # nested include from defs
misc.o:	$(INC)/sys/errno.h # nested include from errno.h
misc.o:	$(INC)/sys/signal.h # nested include from signal.h
misc.o:	$(INC)/sys/stat.h
misc.o:	$(INC)/sys/types.h
	$(CC_CMD) misc.c

prtmem.o:	prtmem.c
prtmem.o:	$(INC)/stdio.h # nested include from defs
prtmem.o:	$(INC)/sys/dir.h
prtmem.o:	$(INC)/sys/param.h
prtmem.o:	$(INC)/sys/user.h
	$(CC_CMD) prtmem.c

rules.o:	rules.c
rules.o:	$(INC)/stdio.h # nested include from defs
	$(CC_CMD) rules.c

########
#
#	Standard Targets
#
#	all		builds all the products specified by PRODUCTS
#	install		installs products; user defined in make.lo 
#	clean		removes all temporary files (ex. installable object)
#	clobber		"cleans", and then removes $(PRODUCTS)
#	remove		remove all installed files
#	product		lists all files installed
#	productdir	lists all required directories for this component
#	partslist	produces a list of all source files used in the
#			  construction of the component (including makefiles)
#	strip		allows for the stripping of the final executable
#	makefile	regenerates makefile
#
########

install: 	# rules, if any, specified above

clean:
		-rm -f $(CLEAN) $(AUX_CLEAN)

clobber:	clean
		-rm -f $(PRODUCTS)

partslist:
		@echo $(SOURCES)

product:
		@echo $(PRODUCTS)

strip:

makefile:	$(MAKE.LO) $(MAKE.ROOT)
		$(MKGEN) >make.out
		if [ -s make.out ]; then mv make.out makefile; fi

makefile_all:	makefile

save:
	cp $(CCSBIN)/make $(CCSBIN)/sv.make
