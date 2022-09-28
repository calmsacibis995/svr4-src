#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)tplot:tplot.mk	1.8.1.4"
#	tplot make file

ROOT =
OL = $(ROOT)/
SL = $(ROOT)/usr/src/cmd
INSDIR = $(OL)usr/bin
INSLIB = $(OL)usr/lib
RDIR = $(SL)/tplot
REL = current
LIST = lp
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O $(FFLAG) -I$(INC)
LDFLAGS = -s
SOURCE = banner.c driver.c tplot.sh vplot.c
MAKE = make

all: t4014 t300 t300s t450 vplot tplot

t4014: driver.o
	$(CC) $(LDFLAGS) -o t4014 driver.o -l4014 -lm $(SHLIBS)

t300:	driver.o
	$(CC) $(LDFLAGS) -o t300 driver.o -l300 -lm $(SHLIBS)

t300s: driver.o
	$(CC) $(LDFLAGS) -o t300s driver.o -l300s -lm $(SHLIBS)

t450:	driver.o
	$(CC) $(LDFLAGS) -o t450 driver.o -l450 -lm $(SHLIBS)

vplot:	vplot.o banner.o
	$(CC) $(LDFLAGS) -o vplot vplot.o banner.o $(SHLIBS)

tplot:	tplot.sh
	cp tplot.sh tplot

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin tplot
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin t4014
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin vplot
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin t450
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin t300s
	$(INS) -f $(INSLIB) -m 0555 -u bin -g bin t300

build:	bldmk
	get -p -r`gsid tplot current` s.tplot.src $(REWIRE) | ntar -d $(RDIR) -g

bldmk:
	get -p -r`gsid tplot.mk current` s.tplot.mk > $(RDIR)/tplot.mk

listing:
	pr tplot.mk $(SOURCE) | $(LIST)

listmk:
	pr tplot.mk | $(LIST)

edit:
	get -e -p s.tplot.src | ntar -g

delta:
	ntar -p $(SOURCE) > tplot.src
	delta s.tplot.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.tplot.mk
mkdelta: ;  delta s.tplot.mk

clean:
	 -rm -f *.o

clobber: clean
	 -rm -f t4014 t300 t300s t450 vplot tplot

delete:	clobber
	-rm -f $(SOURCE)
