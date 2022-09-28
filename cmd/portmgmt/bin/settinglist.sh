#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/settinglist.sh	1.1.2.1"

i=$1
auto=`grep "^$i *:" /etc/ttydefs | cut -d: -f4`
if [ "$auto" = "A" ]
then
	autob=yes
else
	autob=no
fi
echo
ed - /etc/ttydefs <<-!
	H
	v/^${i} *:/ d
	s/^/Ttylabel -	/
	s/ *: */:/g
	s/  */ /g
	s/:/\\
	\Initial Flags -	/
	s/:/\\
	\Final Flags -	/
	s/:.*:/\\
	\Autobaud -	$autob:/
	s/:/\\
	\Next Label -	/
	\$a

	.
	,p
	v/ Flags -	/d
	,s/.* Flags -	//
	,s/[ 	][ 	]*/\\
	/g
	,g/^[ 	]*\$/d
	,s;.*;/^&	/;
	w !sort -u|ed - $OBJ_DIR/../ttyvalues
!
echo
