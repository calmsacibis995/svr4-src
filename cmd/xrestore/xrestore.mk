#ident	"@(#)xrestore.mk	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xrestore:xrestore.mk	1.3"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#
#	@(#) xrestore.mk 1.3 88/05/16 xrestore:xrestore.mk
#


# flags
CFLAGS	= $(GCFLAGS)
LDFLAGS	= $(GLDFLAGS) -s
LIBS	= $(LDLIBS) -lcmd

# commands
CMD	= echo

# directories
DBIN	= $(ROOT)/usr/bin
DLIB	= $(ROOT)/lib
DETC	= $(ROOT)/etc

# objects
EXES	= xrestor
OBJS	= restor.o
SRCS	= restor.c

# standard targets
all :	exes libs

install: all
	-[ -d $(DBIN) ] || mkdir $(DBIN)
	cpset xrestor $(DBIN)
	-rm -f $(DBIN)/xrestore
	ln $(DBIN)/xrestor $(DBIN)/xrestore
	-[ -d $(DETC)/default ] || mkdir $(DETC)/default
	cp restor.dfl $(DETC)/default/xrestor

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(EXES)

cmd:
	$(CMD) $(SRCS)

FRC:

# application targets
$(OBJS):	$(FRC)

exes:	$(EXES)

libs:	restor.dfl


xrestor:		restor.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o xrestor restor.o $(LIBS) 
