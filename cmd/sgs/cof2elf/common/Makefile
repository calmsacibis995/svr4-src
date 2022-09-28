#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cof2elf:common/Makefile	1.1"

ROOT		=
SGS		= 
OWN		= 
GRP		= 
CC		= $(PFX)cc
CCSBIN		= $(ROOT)/usr/ccs/bin
CCSLIB		= $(ROOT)/usr/ccs/lib
CFLAGS		= 
COMDIR		= ../common
ELFDIR		= $(SGSBASE)/inc/common
INS		= $(SGSBASE)/sgs.install
LDLIBS		=
LINK_MODE	=
LINT		= 
LINTFLAGS	=
SGSBASE		= ../..
STRIP		= strip

SOURCES		= \
		$(COMDIR)/cof2elf.c

OBJECTS		= \
		cof2elf.o

PRODUCTS	= cof2elf

COMINC		= $(SGSBASE)/inc/common
INC		= 
INCSYS		= 
INCPATH		= $(SGSBASE)/inc/$(MACH)

DEFLIST		= -DPORTAR
LIBELF 		= $(SGSBASE)/libelf/$(MACH)/libelf.a

INCLIST		= -I$(INCPATH) -I$(COMINC) -I$(COMDIR) -I./

CC_CMD		= $(CC) $(CFLAGS) $(DEFLIST) $(INCLIST)
COM_CC_CMD	= $(CC_CMD) -c $(COMDIR)/$(@:.o=.c)
SGS_CC_CMD	= $(CC_CMD) -c $(@:.o=.c)

all:		$(PRODUCTS)

install:	all
	cp cof2elf cof2elf.bak
	$(STRIP) cof2elf
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(CCSBIN)/$(SGS)cof2elf cof2elf
	mv cof2elf.bak cof2elf

lintit:		$(SOURCES)
	$(LINT) $(LINTFLAGS) $(INCLIST) $(SOURCES) $(LIBELF)

clean:
	-rm -f $(OBJECTS)

shrink:		clean
	-rm -f $(PRODUCTS) 

clobber: shrink

cof2elf: $(OBJECTS) 
	$(CC_CMD) $(LINK_MODE) -o $@ $(OBJECTS) $(LIBELF) $(LDLIBS)


cof2elf.o:	$(COMDIR)/cof2elf.c
	$(COM_CC_CMD)
