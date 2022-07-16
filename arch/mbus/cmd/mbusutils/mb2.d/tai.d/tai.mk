#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/tai.d/tai.mk	1.3"

# Make file to install tai example programs. NOTE that these programs
# are examples only, and are not intended for production use.

DIR  = $(ROOT)/usr/lib/tai
EDIR  = $(ROOT)/usr/lib/tai/example

all: 

install: all 
	[ -d $(DIR) ] || mkdir $(DIR)
	[ -d $(EDIR) ] || mkdir $(EDIR)
	-@ cd examples; for i in *					;\
	do								 \
		install -m 644 -f $(EDIR) $$i				;\
	done

clean:

clobber:
