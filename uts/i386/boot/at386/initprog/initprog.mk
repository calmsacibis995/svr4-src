#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)boot:boot/at386/initprog/initprog.mk	1.1.5.1"

ASFLAGS = $(MORECPP)
CFLAGS = -O -I$(INCRT) $(MORECPP)
INSDIR = $(ROOT)/etc/initprog

.s.o:
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${CPP} -P ${ASFLAGS} $*.s $*.i ; \
	else \
		${CC} -P ${ASFLAGS} $*.s ; \
	fi
	${AS} -o $*.o $*.i 
	-/bin/rm $*.i  

all: 	compaq att at386

compaq: compaq.o 
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${LD} -Mbinimapfile -dn -e initprog -o compaq compaq.o ; \
	else \
		${LD} -N -e initprog -o compaq compaq.o ; \
	fi 
#	../tool/doitinit compaq

att: att.o misc.o inout.o video.o
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${LD} -Mbinimapfile -dn -e initprog -o att att.o misc.o inout.o video.o ; \
	else \
		${LD} -N -e initprog -o att att.o misc.o inout.o video.o ; \
	fi 
#	../tool/doitinit att

at386: at386.o misc.o inout.o
	if [ x${CCSTYPE} = xELF ] ; \
	then \
		${LD} -Mbinimapfile -dn -e initprog -o at386 at386.o misc.o inout.o ; \
	else \
		${LD} -N -e initprog -o at386 at386.o misc.o inout.o ; \
	fi
#	../tool/doitinit at386

clean:
	-/bin/rm *.o 

clobber: clean
	-/bin/rm at386 att compaq 

install: all
	-mkdir $(INSDIR)
	cpset at386 $(INSDIR) 644 bin bin 
	cpset att $(INSDIR) 644 bin bin 
	cpset compaq $(INSDIR) 644 bin bin 
