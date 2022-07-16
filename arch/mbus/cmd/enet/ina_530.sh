#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

:
#
#	Copyright 1988 Intel Corporation
#	All Rights Reserved
#

#ident	"@(#)mbus:cmd/enet/ina_530.sh	1.3.1.1"

# ina:	Download Ethernet Controller with iNA961 Software
#
#	The file "ina" is invoked from the S08ina961
#	SV-OpenNET startup script in /etc/rc2.d.
#
#
#ident	"@(#)ina  $SV_enet R4.0 - 05/16/90$#
PATH=$PATH:/usr/lib/cci:/etc/conf/bin:/sbin
if [ "$2" = "186/530" ]; then
	lckld /etc/ina961.36 $1
elif [ "$2" = "MIX386/560" ]; then
	lckld /etc/ina961.36.M $1
fi
