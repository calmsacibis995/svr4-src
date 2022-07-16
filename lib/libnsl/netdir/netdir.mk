#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libnet:netdir/netdir.mk	1.9.1.1"

# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# 		PROPRIETARY NOTICE (Combined)
# 
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
# 
# 
# 
# 		Copyright Notice 
# 
# Notice of copyright on this source code product does not indicate 
# publication.
# 
# 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
# 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
# 	          All rights reserved.
 
ROOT =
INC = $(ROOT)/usr/include
CFLAGS = -O -Kpic -I$(INC)
STRIP = strip
SIZE = size
LINT = lint
OBJECTS = netdir.o 
LIBOBJECTS = ../netdir.o 
SRCS = $(OBJS:%.o=%.c)

all:	$(OBJECTS)
	if [ x$(CCSTYPE) != xCOFF ] ; \
	then \
		cp $(OBJECTS) ../ ; \
	fi

netdir.o:	netdir.c \
		$(INC)/netdir.h \
		$(INC)/stdio.h \
		$(INC)/tiuser.h \
		$(INC)/netconfig.h \
		$(INC)/string.h \
		$(INC)/sys/types.h \
		$(INC)/sys/file.h 

lint: 
	$(LINT) $(CFLAGS) $(SRCS)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(LIBOBJECTS)

size: all
	$(SIZE) $(LIBOBJECTS)

strip: all
	$(STRIP) $(LIBOBJECTS)


