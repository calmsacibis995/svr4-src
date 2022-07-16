#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucbplot:plot.mk	1.2.3.1"

#       Copyright(c) 1988, Sun Microsystems, Inc.
#       All Rights Reserved


CFLAGS=	-O
LDFLAGS = $(SHLIBS) -s
ALL =	tek t4013 t300 t300s t450 aedplot bgplot dumbplot gigiplot \
	hpplot hp7221plot implot atoplot plottoa vplot crtplot plot

all:	${ALL} debug 

tek:	libplot/libt4014.a driver.o
	${CC} -o tek  driver.o libplot/libt4014.a -lm $(LDFLAGS)

t4013:	libplot/libt4013.a driver.o
	${CC} -o t4013  driver.o libplot/libt4013.a -lm $(LDFLAGS)

t300:	libplot/libt300.a driver.o 
	${CC} -o t300 driver.o libplot/libt300.a -lm $(LDFLAGS)

t300s:	libplot/libt300s.a driver.o 
	${CC} -o t300s driver.o libplot/libt300s.a -lm $(LDFLAGS)

t450:	libplot/libt450.a driver.o 
	${CC} -o t450 driver.o libplot/libt450.a -lm $(LDFLAGS)

vplot:	vplot.o chrtab.o
	${CC} -o vplot vplot.o chrtab.o $(LDFLAGS)
crtplot:	crtplot.o crtdriver.o
	${CC} -o crtplot crtplot.o crtdriver.o -lcurses -lm $(LDFLAGS)

aedplot: libplot/libaed.a driver.o
	${CC} -o aedplot driver.o libplot/libaed.a $(LDFLAGS)

bgplot: libplot/plotbg.a driver.o
	${CC} -o bgplot driver.o libplot/plotbg.a -lm $(LDFLAGS)

dumbplot: libplot/libdumb.a driver.o
	${CC} -o dumbplot driver.o libplot/libdumb.a -ltermcap -lm $(LDFLAGS)

gigiplot: libplot/libgigi.a driver.o
	${CC} -o gigiplot driver.o libplot/libgigi.a -lm $(LDFLAGS)

hpplot: libplot/libhp2648.a driver.o
	${CC} -o hpplot driver.o libplot/libhp2648.a -lm $(LDFLAGS)

hp7221plot: libplot/libhp7221.a driver.o
	${CC} -o hp7221plot driver.o libplot/libhp7221.a -lm $(LDFLAGS)

implot: libplot/libimagen.a driver.o
	${CC} -o implot driver.o libplot/libimagen.a -lm $(LDFLAGS)

atoplot: libplot/libplot.a atoplot.o
	${CC} -o atoplot atoplot.o libplot/libplot.a -lm $(LDFLAGS)

plottoa: plottoa.o
	${CC} -o plottoa plottoa.o $(LDFLAGS)

libplot/libt300.a:
	cd libplot; make -f libplot.mk lib300

libplot/libt300s.a: 
	cd libplot; make -f libplot.mk lib300s

libplot/libt450.a: 
	cd libplot; make -f libplot.mk lib450

libplot/libt4014.a: 
	cd libplot; make -f libplot.mk lib4014

libplot/libaed.a:
	cd libplot; make -f libplot.mk libplotaed

libplot/plotbg.a:
	cd libplot; make -f libplot.mk libplotbg

libplot/libdumb.a:
	cd libplot; make -f libplot.mk libplotdumb

libplot/libf77plot.a:
	cd libplot; make -f libplot.mk libf77plot

libplot/libgigi.a:
	cd libplot; make -f libplot.mk libplotgigi

libplot/libhp2648.a:
	cd libplot; make -f libplot.mk libplot2648

libplot/libhp7221.a:
	cd libplot; make -f libplot.mk libplot7221

libplot/libimagen.a:
	cd libplot; make -f libplot.mk libplotimagen

libplot/libplot.a:
	cd libplot; make -f libplot.mk libplot

libplot/libt4013.a:
	cd libplot; make -f libplot.mk lib4013

plot: plot.sh
	cp plot.sh plot

debug: debug.o
	${CC} -o debug debug.o $(LDFLAGS)

install: all plot
	-for i in ${ALL}; do \
		(install -f  ${ROOT}/usr/ucb -m 0555 -u bin -g bin $$i); done

clean:
	cd libplot; make -f libplot.mk clean
	rm -f *.o a.out core errs

clobber: 
	cd libplot; make -f libplot.mk clobber
	rm -f *.o a.out core errs
	rm -f ${ALL} debug
