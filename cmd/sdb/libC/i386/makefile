#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:libC/i386/makefile	1.2"

OBJECTS = _delete.o _handler.o _new.o _vec.o \
	_main.o exit.o _ctor.o _dtor.o dtors.o

include ../../util/defs.make

TARGET = $(PRODLIB)/libC.a

all:	$(TARGET)

$(TARGET):	$(OBJECTS)
	rm -f $(TARGET)
	$(AR) -q $(TARGET) $(OBJECTS)

install:	all

depend:
	@: ignored

_delete.o:	$(COMMON)/_delete.C
	$(CPLUS) -c $(COMMON)/_delete.C

_handler.o:	$(COMMON)/_handler.C
	$(CPLUS) -c $(COMMON)/_handler.C

_new.o:	$(COMMON)/_new.C
	$(CPLUS) -c $(COMMON)/_new.C

_vec.o:	$(COMMON)/_vec.C
	$(CPLUS) -c $(COMMON)/_vec.C

_main.o:	$(COMMON)/_main.C
	$(CPLUS) -c $(COMMON)/_main.C

_ctor.o:	$(COMMON)/_ctor.C
	$(CPLUS) -c $(COMMON)/_ctor.C

_dtor.o:	$(COMMON)/_dtor.C
	$(CPLUS) -c $(COMMON)/_dtor.C

exit.o:	$(COMMON)/exit.C
	$(CPLUS) -c $(COMMON)/exit.C

dtors.o:	$(COMMON)/dtors.C
	$(CPLUS) -c $(COMMON)/dtors.C

clean:
	rm -f *.o

clobber:	clean
	rm -f $(TARGET)
