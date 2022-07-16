#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1988  Intel Corporation
#	All Rights Reserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/initbp.sh	1.3"

# Initialize the bootstrap parameter string on a board in the specified 
# slot.

Do_usage (){
	echo "Usage: initbp [-v] slot_number"
	exit 1
}

PNAME=/usr/lbin
OPTIONSTR=""
while getopts v options
do
	case $options in
		v)	OPTIONSTR="-v" ;;
		\?)	Do_usage ;;
	esac
done
shift `expr $OPTIND - 1`

SLOT=$1
[ -z "$SLOT" ] && Do_usage
OPTIONSTR="$OPTIONSTR -r"
$PNAME/reset $OPTIONSTR $SLOT
