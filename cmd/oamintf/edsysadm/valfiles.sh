#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/valfiles.sh	1.3.2.1"

################################################################################
#	Module Name: valfiles
#
#	Inputs: 
#		$1 -> comma separated list of task files (Task only)
#
#	Output:
#		if a file does not exist it is echoed to stdout
#
#	Description:
#		with 'echo' and 'cut' cycle through all files entered
#		in Form.task and validate ( using 'valpath' ) that each
#		one exist.  If a file does not exist echo the file to
#		stdout and exit.
################################################################################

EXIT=0

# blank is a valid entry.
if [ ! -n "$1" ]
then
 	exit 0
fi

if [ "." = "$1" ]
then
dot=`pwd`
	for b in `cat $OAMBASE/menu/resrv.fs`
	do
		if [ "$b" = "$dot" ]
		then
			exit 1
		fi
	done
	exit 0
fi
	

files=`echo $1 | sed -e 's/,/ /g'`

for x in $files
do
	valpath -o "$x"

	if [ $? = 1 ]
	then
		echo $x >> /tmp/${VPID}.nf
		EXIT=1
	else
		for f in `cat $OAMBASE/menu/resrv.fs`
		do
			if [ "$f" = "$x" ]
			then
				exit 1
			fi
		done
	fi

	continue
done

exit $EXIT
