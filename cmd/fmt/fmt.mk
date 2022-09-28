#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Portions Copyright (c) 1988, Sun Microsystems, Inc.
# 	All Rights Reserved.

#ident	"@(#)fmt:fmt.mk	1.3.1.1"

#     Makefile for fmt 

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LOCHDR = hdr

LDFLAGS = -s $(SHLIBS) 

CFLAGS = -O -I$(INC) -I$(LOCHDR)

STRIP = strip

SIZE = size

#top#

MAKEFILE = fmt.mk

MAINS = fmt 

OBJECTS =  fmt.o head.o

SOURCES =  fmt.c head.c

ALL:          $(MAINS)

$(MAINS):	fmt.o head.o
	$(CC) $(CFLAGS) -o fmt fmt.o head.o $(LDFLAGS)
	
fmt.o:		$(INC)/stdio.h \
		$(INC)/ctype.h

head.o:		$(LOCHDR)/def.h $(LOCHDR)/glob.h \
		$(LOCHDR)/local.h $(INC)/stdio.h \
		$(LOCHDR)/v7.local.h $(LOCHDR)/usg.local.h \
		$(INC)/sys/param.h $(INC)/signal.h \
		$(INC)/stdio.h 

GLOBALINCS = $(INC)/stdio.h $(INC)/ctype.h \
		$(LOCHDR)/local.h $(INC)/stdio.h \
		$(LOCHDR)/v7.local.h $(LOCHDR)/usg.local.h \
		$(INC)/sys/param.h $(INC)/signal.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

all :	ALL

install:	ALL
	install -f $(DIR) -m 00555 -u bin -g bin fmt

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)

#     These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(DIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(DIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)

