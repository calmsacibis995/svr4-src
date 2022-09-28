#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/addform.sh	1.1.1.1"


fname=$1
desc="$2"
cp=$3
lp=$4
pl=$5
pw=$6
np=$7


(echo "page length:" $pl\\n"page width:" $pw\\n"line pitch:" $lp\\n"character pitch:" $cp\\n"number of pages:" $np ) > /usr/tmp/form.$VPID


shift 7
	
if  [ "$1" != "" -a "$2" = "No" ]
then 
	echo "character set choice:" $1 >> /usr/tmp/form.$VPID
elif [ "$1" != "" -a "$2" = "Yes" ]
then 
echo "character set choice:" $1,mandatory >>  /usr/tmp/form.$VPID
fi


if [ "$3" != "" ]
then echo "ribbon color:" "$3" >>  /usr/tmp/form.$VPID
fi

if [ ! -z "$desc" ]
then
echo "comment:" >> /usr/tmp/form.$VPID
echo "$desc" >>/usr/tmp/form.$VPID
fi


altype=$5 

if [ "$4" != "" -a "$altype" != "" ]
	then
	echo "alignment pattern: $altype" >> /usr/tmp/form.$VPID
	cat "$4" >> /usr/tmp/form.$VPID
	echo "$fname: $4" >> /usr/vmsys/OBJECTS/PS/FORM/alnames
fi


atype=$6 
freq=""
npreq=""

if [ "$atype" != "none" ]
	then 
	if [ "$7" = "once" ]
	then
		freq=0
	else
		freq=$7
	fi
	npreq=$8
fi

shift 8

users=`echo "$1" | tr '\012' ' '`
users=`echo "$users"`

if [ "$atype" != "none" ]
then

  if [ "$atype" = "mail" ] ; then
  /usr/lib/lpforms -f $fname -F /usr/tmp/form.$VPID -u allow:"$users"  -A "mail $LOGNAME" -W $freq -Q $npreq > /dev/null
  else
  /usr/lib/lpforms -f $fname -F /usr/tmp/form.$VPID -u allow:"$users"  -A "$atype" -W $freq -Q $npreq > /dev/null
  fi

elif [ "$atype" = "none" ]
then
/usr/lib/lpforms -f $fname -F /usr/tmp/form.$VPID -u allow:"$users" -A none > /dev/null
fi


rm -rf /usr/tmp/form.$VPID
echo 0
