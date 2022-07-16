#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:io/Makefile	1.4"
#************************************************************************
#*	Copyright (c) 1988, 1989 ARCHIVE Corporation			*
#*	This program is an unpublished work fully protected by the	*
#*	United States Copyright laws and is considered a trade secret	*
#*	belonging to Archive Corporation.				*
#************************************************************************
#
CFLAGS=		-O 
OFILES=		xl.o xl_dec.o xla.o


all:		$(OFILES) date.o fd.o
		$(LD) -r $(OFILES) date.o -o Driver.o
		$(LD) -r fd.o -o Driver.o.fd

install package:	all 
		-ls Driver.o* | cpio -pduv ../pkg

fd.o:		fd.c $(FRC)
		$(CC) -c $(CFLAGS) fd.c

xl.o:		xl.c xl.h $(FRC)
		$(CC) -c $(CFLAGS) xl.c

date.o:		date.c

xl_dec.o:	xl_dec.c xl.h

date.c:		$(OFILES)
		rm -f date.c
		echo "char	compdate[]=\"`date`\";" > date.c

xla.s:		xla.orig.s
		rm -rf xla.s
		awk -F, -f awk.sh xla.orig.s > xla.s

clean: 		$(FRC)
		rm -f *.o  Driver.o.fd

clobber: 	clean
		rm -f xla.s date.c

FRC:
