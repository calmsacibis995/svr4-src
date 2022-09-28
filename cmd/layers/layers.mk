#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)attwin:cmd/layers/makefile	1.13.5.1"
#
#		Copyright 1985 AT&T
#

CC = cc
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC) -Di386 -DSVR40
INS = install
STRIP = strip

all:	deodx wtinit_mk misc_mk layers layersys/lsys.873 set_enc.j relogin xts xtt 

layers:	layers.o xtraces.o xtstats.o
	$(CC) $(CFLAGS) -o layers layers.o xtraces.o xtstats.o $(SHLIBS) -lwindows

relogin:	relogin.c
relogin:	$(INC)/sys/types.h
relogin:	$(INC)/utmp.h
relogin:	$(INC)/stdio.h
relogin:	$(INC)/pwd.h
	$(CC) $(CFLAGS) -o relogin relogin.c $(SHLIBS)

xts:	xts.o xtstats.o
	$(CC) $(CFLAGS) -o xts xts.o xtstats.o $(SHLIBS)

xtt:	xtt.o xtraces.o
	$(CC) $(CFLAGS) -o xtt xtt.o xtraces.o $(SHLIBS)

layers.o:	layers.c
layers.o:	$(INC)/sys/types.h
layers.o:	$(INC)/sys/stropts.h
layers.o:	$(INC)/sys/stream.h
layers.o:	$(INC)/ctype.h
layers.o:	$(INC)/sys/stat.h
layers.o:	$(INC)/sys/errno.h
layers.o:	$(INC)/signal.h
layers.o:	$(INC)/sys/jioctl.h
layers.o:	$(INC)/sys/nxtproto.h
layers.o:	$(INC)/sys/termio.h
layers.o:	$(INC)/fcntl.h
layers.o:	$(INC)/stdio.h
layers.o:	sxtstat.h
layers.o:	$(INC)/sys/nxt.h
layers.o:	$(INC)/sys/tty.h

xts.o:	xts.c
xts.o:	$(INC)/stdio.h
xts.o:	$(INC)/errno.h
xts.o:	$(INC)/sys/param.h
xts.o:	$(INC)/sys/types.h
xts.o:	$(INC)/ctype.h
xts.o:	$(INC)/sys/tty.h
xts.o:	$(INC)/sys/jioctl.h
xts.o:	$(INC)/sys/stream.h
xts.o:	$(INC)/sys/stropts.h
xts.o:	$(INC)/sys/nxtproto.h
xts.o:	$(INC)/sys/nxt.h

xtt.o:	xtt.c
xtt.o:	$(INC)/sys/types.h
xtt.o:	$(INC)/sys/tty.h
xtt.o:	$(INC)/sys/jioctl.h
xtt.o:	$(INC)/sys/stropts.h
xtt.o:	$(INC)/sys/stream.h
xtt.o:	$(INC)/sys/nxtproto.h
xtt.o:	$(INC)/sys/nxt.h
xtt.o:	$(INC)/stdio.h

xtraces.o:	xtraces.c
xtraces.o:	$(INC)/stdio.h
xtraces.o:	$(INC)/sys/param.h
xtraces.o:	$(INC)/sys/types.h
xtraces.o:	$(INC)/ctype.h
xtraces.o:	$(INC)/sys/tty.h
xtraces.o:	$(INC)/sys/jioctl.h
xtraces.o:	$(INC)/sys/stropts.h
xtraces.o:	$(INC)/sys/stream.h
xtraces.o:	$(INC)/sys/nxtproto.h
xtraces.o:	$(INC)/sys/nxt.h

xtstats.o:	xtstats.c
xtstats.o:	$(INC)/sys/types.h
xtstats.o:	$(INC)/sys/tty.h
xtstats.o:	$(INC)/sys/jioctl.h
xtstats.o:	$(INC)/sys/stropts.h
xtstats.o:	$(INC)/sys/stream.h
xtstats.o:	$(INC)/sys/nxtproto.h
xtstats.o:	$(INC)/sys/nxt.h
xtstats.o:	$(INC)/stdio.h

set_enc.j:
	./deodx < set_enc.j.src > set_enc.j 

layersys/lsys.873:
	./deodx < layersys/lsys.873.src > layersys/lsys.873

deodx:	deodx.c
	if [ -f /bin/cc ];\
	then /bin/cc -O -s -o deodx deodx.c;\
	elif [ -f /usr/ccs/bin/cc ];\
	then /usr/ccs/bin/cc -O -s -o deodx deodx.c;\
	else cc -O -s -o deodx deodx.c;\
	fi

wtinit_mk:
	cd wtinit; $(MAKE) -f wtinit.mk

misc_mk:
	cd misc; $(MAKE) -f misc.mk

install:	all
	if [ ! -d $(ROOT)/usr/lib/layersys ] ;\
	then \
		mkdir $(ROOT)/usr/lib/layersys ;\
		$(CH)chmod 755 $(ROOT)/usr/lib/layersys ;\
		$(CH)chgrp bin $(ROOT)/usr/lib/layersys ;\
		$(CH)chown bin $(ROOT)/usr/lib/layersys ;\
	fi 
	$(INS) -f $(ROOT)/usr/bin -u root -g bin -m 4755 layers
	$(STRIP) $(ROOT)/usr/bin/layers
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 xts
	$(STRIP) $(ROOT)/usr/bin/xts
	$(INS) -f $(ROOT)/usr/bin -u bin -g bin -m 755 xtt
	$(STRIP) $(ROOT)/usr/bin/xtt
	$(INS) -f $(ROOT)/usr/lib/layersys -u root -g bin -m 4755 relogin
	$(STRIP) $(ROOT)/usr/lib/layersys/relogin
	cp layersys/lsys.873 $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chown bin $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chgrp bin $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(CH)chmod 755 $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
# Don't strip ELF has problems.
#	$(STRIP) $(ROOT)/usr/lib/layersys/lsys.8\;7\;3
	$(INS) -f $(ROOT)/usr/lib/layersys -u bin -g bin -m 755 set_enc.j
# Don't strip ELF has problems.
#	$(STRIP) $(ROOT)/usr/lib/layersys/set_enc.j
	cd wtinit; $(MAKE) -f wtinit.mk install
	cd misc; $(MAKE) -f misc.mk install

clean:
	rm -f layers.o xts.o xtt.o xtraces.o xtstats.o 
	cd wtinit; $(MAKE) -f wtinit.mk clean
	cd misc; $(MAKE) -f misc.mk clean

clobber:
	rm -f layers.o xts.o xtt.o xtraces.o xtstats.o layers relogin xts xtt
	rm -f layersys/lsys.873 set_enc.j deodx
	cd wtinit; $(MAKE) -f wtinit.mk clobber
	cd misc; $(MAKE) -f misc.mk clobber
