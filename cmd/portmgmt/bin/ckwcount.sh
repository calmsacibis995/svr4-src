#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/ckwcount.sh	1.1.2.1"

# ckwcount - check if the wait read count is valid
#	   Input:	$1 - wait read flag (Yes/No)
#			$2 - wait read count 

OK=0		
NOTVALIDYES=1	# not valid for waitread=Yes
NOTVALIDNO=2	# not valid for waitread=No
NOTHING=3	# nothing entered or wrong args

case $1 in
	Yes)
		test -z "$2" && exit $NOTVALIDYES
		num=`expr "$2" + 0 2>/dev/null`
		test -z "$num" && exit $NOTVALIDYES
		[ $num -lt 0 ] && exit $NOTVALIDYES
		exit $OK;;
	No)	
		test -z "$2" && exit $OK
		exit $NOTVALIDNO;;
	*)	exit $NOTHING;;
		
esac
