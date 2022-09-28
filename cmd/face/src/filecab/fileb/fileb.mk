#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/fileb.mk	1.10"

USR=$(ROOT)/$(HOME)

MKDIR	=	mkdir
VBIN=$(USR)/vmsys/bin
VLIB=$(USR)/vmsys/lib
OABIN = $(USR)/oasys/bin
HEADER1=../inc
LIBPW=$(LIBPW)
INCLUDE=	-I$(HEADER1)

DIRS= $(USR)/vmsys $(USR)/oasys

CFLAGS= -O 

LDFLAGS =  -s

LCMDS = services

OCMDS =	termtest identify setenv

VCMDS = face dir_move dir_copy dir_creat chkperm creaserve listserv delserve ichexec chexec chkterm basename mnucheck modserv color_chk frame_chk

CMDS = $(VCMDS) $(OCMDS)

face: face.o
	$(CC) $(LDFLAGS) -o $@ face.o -l$(LIBPW) $(SHLIBS)

face.o:	face.c 

chkperm: chkperm.o
	$(CC) $(LDFLAGS) -o $@ chkperm.o $(SHLIBS)

chkperm.o: chkperm.c $(HEADER1)/wish.h

creaserve: creaserve.o
	$(CC) $(LDFLAGS) -o $@ creaserve.o $(SHLIBS)

creaserve.o: creaserve.c $(HEADER1)/wish.h

delserve: delserve.o
	$(CC) $(LDFLAGS) -o $@ delserve.o $(SHLIBS)

delserve.o: delserve.c $(HEADER1)/wish.h

listserv: listserv.o
	$(CC) $(LDFLAGS) -o $@ listserv.o $(SHLIBS)

listserv.o: listserv.c $(HEADER1)/wish.h

modserv: modserv.o
	$(CC) $(LDFLAGS) -o $@ modserv.o $(SHLIBS)

modserv.o: modserv.c $(HEADER1)/wish.h

mnucheck: mnucheck.o
	$(CC) $(LDFLAGS) -o $@ mnucheck.o $(SHLIBS)

mnucheck.o: mnucheck.c $(HEADER1)/wish.h


termtest:	termtest.c $(HEADER1)/wish.h
	$(CC) $(CFLAGS) $(LDFLAGS)  $(INCLUDE) -o $@ termtest.c -l$(LIBPW) $(SHLIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

.o:
	$(CC) $(LDFLAGS) -o $@ $? $(SHLIBS)

.sh:
	/bin/rm -f $@
	/bin/cp $< $@
	chmod 755 $@

###### Standard Makefile Targets ######

all:	$(CMDS)

install: all dirs $(VBIN) $(OABIN) $(VLIB)
	@set +e;\
	> $(VLIB)/.facerc
	chmod 600 $(VLIB)/.facerc
	for f in $(LCMDS);\
	do\
		install -f $(VLIB) $$f;\
		chmod 644 $(VLIB)/$$f;\
	done;\
	for f in $(VCMDS);\
	do\
		install -f $(VBIN) $$f;\
		chmod 755 $(VBIN)/$$f;\
	done;\
	for f in $(OCMDS);\
	do\
		install -f $(OABIN) $$f;\
		chmod 755 $(OABIN)/$$f;\
	done;\
	chmod 6755 $(VBIN)/chkperm

dirs:	$(DIRS)

$(DIRS):
	$(MKDIR) $@ 
	chmod 755 $@

$(VBIN) $(OABIN) $(VLIB):
	$(MKDIR) $@ 
	chmod 755 $@

clean: 
	@set +e;\
	for f in $(VCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	for f in $(OCMDS);\
	do\
		/bin/rm -f $$f;\
	done;\
	/bin/rm -f *.o

clobber:	clean
