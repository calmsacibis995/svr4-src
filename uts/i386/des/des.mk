#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-des:des.mk	1.1"

#
#	@(#)des.mk 1.1 88/12/14 SMI
#
#
#  		PROPRIETARY NOTICE (Combined)
#  
#  This source code is unpublished proprietary information
#  		Copyright Notice 
#  
#  Notice of copyright on this source code product does not indicate 
#  publication.
#  
#  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
#  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#  	          All rights reserved.
#
#
#	Kernel DES
#
STRIP = strip
INCRT = ..
OFILE = $(CONF)/pack.d/des/Driver.o
DFILE = $(CONF)/pack.d/des/Driver_d.o
IFILE = $(CONF)/pack.d/des/Driver_i.o
PFLAGS = -I$(INCRT) -D_KERNEL $(MORECPP)
CFLAGS = $(DASHO) $(PFLAGS) -DSYSV
DEFLIST =
FRC =

DESOBJ =  des_crypt.o des_soft.o
IDESOBJ =  intldescrypt.o intldes_soft.o


ALL:		
		[ -d $(CONF)/pack.d/des ] || mkdir $(CONF)/pack.d/des; \
		if [ -s des_crypt.c -a  -s des_soft.c ] ;\
		then \
			make -f des.mk domestic ;\
		fi
		make -f des.mk intl; \
		rm -f $(OFILE); ln $(IFILE) $(OFILE)

lint:
	lint $(CFLAGS) -Dlint *.c

domestic:	$(DESOBJ)
	$(LD) -r -o $(DFILE) $(DESOBJ)

intl:	$(IDESOBJ)
	$(LD) -r -o $(IFILE) $(IDESOBJ)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILE) $(DFILE) $(IFILE)


#
# Header dependencies -- make depend should build these!
#

des_crypt.o: des_crypt.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/des_crypt.h \
	$(INCRT)/des/des.h

des_soft.o: des_soft.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/des/des.h \
	$(INCRT)/des/softdes.h \
	$(INCRT)/des/desdata.h \
	$(INCRT)/sys/debug.h

intldescrypt.o: intldescrypt.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/rpc/des_crypt.h \
	$(INCRT)/des/des.h

intldes_soft.o: intldes_soft.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/des/des.h \
	$(INCRT)/des/softdes.h \
	$(INCRT)/des/desdata.h \
	$(INCRT)/sys/debug.h
