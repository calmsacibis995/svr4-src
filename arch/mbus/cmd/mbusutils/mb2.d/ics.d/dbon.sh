#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/dbon.sh	1.3"

# shell script to enable the front panel interrupt
# to break the current processor into the debugger.
#
#! /bin/sh

WRITE=/sbin/icswr
READ=/sbin/icsrd
SLOT_ID=/sbin/icsslot
if [ $# = 1 ]
then
	SLOT=$1
else
	SLOT=`$SLOT_ID`
fi
CSM=0
#
# only do below for CSM/001, CSM/002 gets this done via the debugger package
#
if [ `$READ -s $CSM 2 10` = "CSM/001" ];then
	$WRITE $CSM 58 1 $SLOT
fi
#
#
