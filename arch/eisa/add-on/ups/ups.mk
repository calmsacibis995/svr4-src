#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)eisa:add-on/ups/ups.mk	1.3"

ROOT =
MORECPP = -DAT386
INC = $(ROOT)/usr/include
CFLAGS = -O -I../.. -I$(INC) -D_KERNEL $(MORECPP)
FRC =

DRIVER = Driver.o

STRIPFILES = \
	Init \
	Master \
	System \
	Node \
	Name


all:	$(DRIVER) ups_daem
	-for f in $(STRIPFILES); \
	do \
		grep -v "^#" $$f.sh | grep -v "^$$" > $$f; \
	done;

install: all
	
package: install
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	[ -d $(ROOT)/usr/src/pkg/ups.src ] || mkdir $(ROOT)/usr/src/pkg/ups.src
	cat packagelist | cpio -pdum $(ROOT)/usr/src/pkg/ups.src

clean:
	rm -f fpan.o 

clobber: clean
	rm -f $(DRIVER) ups_daem
	rm -f $(STRIPFILES)

Driver.o: fpan.o
	$(LD) -r -o Driver.o fpan.o

fpan.o:		fpan.c \
		$(INC)/sys/types.h \
		$(INC)/sys/param.h \
		$(INC)/sys/sysmacros.h \
		$(INC)/sys/stream.h \
		$(INC)/sys/stropts.h \
		$(INC)/sys/errno.h \
		$(INC)/sys/debug.h \
		$(INC)/sys/xdebug.h \
		$(INC)/sys/cmn_err.h \
		fpan.h
	$(CC) $(CFLAGS) -c fpan.c

FRC:

ups_daem:	ups_daem.c \
		fpan.h 
	cc -o ups_daem ups_daem.c
#	cc -DFPANDEBUG  -o ups_daem ups_daem.c
