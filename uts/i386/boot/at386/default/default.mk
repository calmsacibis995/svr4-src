#ident	"@(#)default.mk	1.2	91/09/15	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)boot:boot/at386/default/default.mk	1.1.5.1"

INSDIR = $(ROOT)/etc/default

install:
	-[ -d $(INSDIR) ] || mkdir $(INSDIR)
	cpset default.at386 $(INSDIR) 644 bin bin
	cpset default.cpq $(INSDIR) 644 bin bin
	cpset default.att $(INSDIR) 644 bin bin
	cpset default.att512 $(INSDIR) 644 bin bin
	cp $(INSDIR)/default.at386 $(INSDIR)/boot

clean:

clobber:

all:
