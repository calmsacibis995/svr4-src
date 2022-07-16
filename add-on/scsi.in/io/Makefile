#ident	"@(#)Makefile	1.2	92/02/17	JPB"

#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:io/Makefile	1.3"

#SYSTEMENV	= 3
#COMPONENTS	= scsi st01 sc01 sw01  sd01 ddi

SYSTEMENV		= 4
COMPONENTS		= scsi st01 sc01 sw01 sd01 
##COMPONENTS 	= scsi st01 sc01 sw01 

MORECPP		= -DAT386 -D_SYSTEMENV=$(SYSTEMENV)
FRC			=

all install clean clobber:
	for comp in $(COMPONENTS); do \
		(cd $$comp; make -e "ROOT=$(ROOT)" "FRC=$(FRC)" "MORECPP=$(MORECPP)" \
			"SYSTEMENV=$(SYSTEMENV)" $@); \
	done

FRC:
