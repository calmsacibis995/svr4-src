#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)libTL:libTL.mk	1.10.3.1"

SRC= TLappend.c TLassign.c TLclose.c TLdelete.c TLgetentry.c TLgetfield.c \
	TLopen.c TLread.c TLsearches.c TLsubst.c TLsync.c TLwrite.c \
	description.c entry.c field.c file.c parse.c search.c space.c \
	table.c utils.c
OBJ=$(SRC:.c=.o)

LIB=$(ROOT)/usr/lib
INC=$(ROOT)/usr/include
LORDER=$(PFX)lorder
CC=$(PFX)cc
AR=ar
LINT=$(PFX)lint
TSORT=$(PFX)tsort

PRODUCT=libTL.a
LOCALHDRS=hdrs

FLAGS=
CFLAGS=-I $(LOCALHDRS) -I $(INC) $(FLAGS)

all: $(PRODUCT)

$(PRODUCT): $(OBJ)
	$(AR) cr $(PRODUCT) `$(LORDER) $(OBJ) | $(TSORT)`

TLtest: $(PRODUCT) TLtest.o
	$(CC) -o $(@) $(CFLAGS) $(LDFLAGS) TLtest.o $(PRODUCT) $(LDLIBS)

touch:
	touch $(SRC)

install: $(PRODUCT)
	install -f $(LIB) -m 0644 $(PRODUCT) 

clean:
	rm -rf $(OBJ)

clobber: clean
	rm -rf $(PRODUCT)

strip: $(PRODUCT)

lintit:
	$(LINT) -u $(CFLAGS) $(SRC)

$(OBJ) TLtest.o : $(INC)/table.h $(LOCALHDRS)/internal.h
