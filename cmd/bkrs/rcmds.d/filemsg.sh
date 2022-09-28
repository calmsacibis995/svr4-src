#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/filemsg.sh	1.1.2.1"
if [ "$2" = "" ]
then
	echo You must provide a valid file or directory name or strike CANCEL.
	exit
fi 

for i in $2
do

	if test -f $i -o -d $i
	then
		:
	else 
		echo "$i cannot be found." 
		exit
	fi

	if [ "$1" = "Personal" ]
	then
		if [ "$i" = "/" -a "$HOME" != "/" ]
		then
			echo "You may only backup files or directories in $HOME."
			exit
		fi

		if test -f $HOME/$i -o -d $HOME/$i
		then
			:
		else
			if echo "$i" | grep "$HOME" > /dev/null
			then
				:
			else
				echo "You may only backup files or directories in $HOME."
				exit
			fi
		fi
	fi
done
