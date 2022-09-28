#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/filechk.sh	1.1.2.1"
TRUE=0
if [ "$2" = "" ]
then
	TRUE=1
	echo $TRUE
	exit
fi

for i in $2
do
	if test -f $i -o -d $i
	then
		:
	else 
		TRUE=1
	fi

# if personal backup check that either the file or directory 
# resides in users $HOME dir or check that user has typed in
# full pathname including $HOME dir.

	if [ "$1" = "Personal" ]
	then
		if [ "$i" = "/" -a "$HOME" != "/" ]
		then
			TRUE=1
			echo $TRUE
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
				TRUE=1
			fi
		fi
	fi
done
echo $TRUE
