#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/ckbaud.sh	1.2.2.1"

# ckbaud - check if the baud rate is available
#	   (ttyvalues contains a list of valid baud rates)
#	   Input:	$1 - autobaud flag (Yes/No)
#			$2 - baud rate

OK=0		
NOTEXIST=1	# not exist
NOTVALID=2	# not valid
NOTHING=3	# nothing entered or wrong args

case $1 in
	Yes)
		test -z "$2" && exit $OK
		exit $NOTVALID;;
	No)	
		test -z "$2" && exit $NOTHING
		if [ "$2" = 0 ]
		then
			exit $NOTEXIST
		fi
		speed=`grep "^$2	" $OBJ_DIR/../ttyvalues 2>/dev/null`
		if test -z "$speed"
		then
			exit $NOTEXIST
		fi
		exit $OK;;
	*)	exit $NOTHING;;
		
esac
