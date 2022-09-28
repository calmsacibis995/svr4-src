#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/mkform.sh	1.1.1.1"

remove=NO
fname=$1
desc="$2"
cp=$3
lp=$4
pl=$5
pw=$6
np=$7

(echo length=$pl\\nwidth=$pw\\nlpi=$lp\\ncpi=$cp\\nnp=$np) > /usr/tmp/chgform.$VPID 

(echo "page length:" $pl\\n"page width:" $pw\\n"line pitch:" $lp\\n"character pitch:" $cp\\n"number of pages:" $np ) > /usr/tmp/uform.$VPID


shift 7
	
if  [ "$1" != "" -a "$2" = "No" ]
then 
	echo "character set choice:" $1 >> /usr/tmp/uform.$VPID
	echo "cs=$1" >> /usr/tmp/chgform.$VPID
	echo "mandatory=No" >> /usr/tmp/chgform.$VPID
elif [ "$1" != "" -a "$2" = "Yes" ]
then 
	echo "character set choice:" $1,mandatory >>  /usr/tmp/uform.$VPID
	echo "cs=$1" >> /usr/tmp/chgform.$VPID
	echo "mandatory=Yes" >> /usr/tmp/chgform.$VPID
fi


if [ "$3" != "" ]
then 
	echo "ribbon color:" "$3" >>  /usr/tmp/uform.$VPID
	echo "rcolor=$3" >>  /usr/tmp/chgform.$VPID
fi

if [ ! -z "$desc" ]
then
	echo "comment:" >> /usr/tmp/uform.$VPID
	echo "$desc" >>/usr/tmp/uform.$VPID
	echo "comment=$desc" >> /usr/tmp/chgform.$VPID
fi


update()
{
	if [ "`grep \"^$1: \" /usr/vmsys/OBJECTS/PS/FORM/alnames`" 2>/dev/null != "" ] 
	then
		ed - /usr/vmsys/OBJECTS/PS/FORM/alnames <<-eof 2>/dev/null 1>&2
		/$1: /
		c
		$1: $2
		.
		w
		q
		eof
	else
		echo "$1: $2" >> /usr/vmsys/OBJECTS/PS/FORM/alnames
	fi
}

delete()
{
	ed - /usr/vmsys/OBJECTS/PS/FORM/alnames <<-eof 2>/dev/null  1>&2
	/$1: /d
	w
	q
	eof
}

altype=$5 

if [ "$4" != "" -a "$altype" != "" ]
	then
	echo "alignment pattern: $altype" >> /usr/tmp/uform.$VPID
	cat "$4" >> /usr/tmp/uform.$VPID
	update $fname $4
	echo "contype=$altype" >> /usr/tmp/chgform.$VPID
elif [ "$4" = "" -a "$altype" = "" ]
	then
	if [ "`grep \"^$fname: \" /usr/vmsys/OBJECTS/PS/FORM/alnames 2>/dev/null`"  != "" ] 
		then remove=YES
	fi
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
if [ "$atype" != "none" ]
then
	echo "fault=$atype" >> /usr/tmp/chgform.$VPID

        [ "$atype" = "mail" ] && echo "login=$LOGNAME" >> /usr/tmp/chgform.$VPID
	echo "freq=$7" >> /usr/tmp/chgform.$VPID
	echo "request=$8" >> /usr/tmp/chgform.$VPID
else
	echo "fault=none" >> /usr/tmp/chgform.$VPID
fi

if [ ! -z "$4" ]
then
	echo "pattern=$4" >> /usr/tmp/chgform.$VPID
fi

shift 8

users=`echo "$1" | tr '\012' ' ' `
users=`echo $users`

echo "users=$users" >> /usr/tmp/chgform.$VPID

/bin/diff /usr/tmp/form.$VPID /usr/tmp/chgform.$VPID 2>/dev/null 1>&2

if [ $? = 0 ]
	then echo 1
	rm -rf /usr/tmp/uform.$VPID /usr/tmp/chgform.$VPID 
	exit
fi

if [ "$remove" = "NO" -a "$atype" != "none" ]
then

  if [ "$atype" = "mail" ] ; then
    /usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -u allow:"$users"  -A "mail $LOGNAME" -W $freq -Q $npreq > /dev/null
  else
    /usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -u allow:"$users"  -A "$atype" -W $freq -Q $npreq > /dev/null
  fi

elif [ "$remove" = "NO" -a "$atype" = "none" ]
then
/usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -u allow:"$users" -A none > /dev/null
elif [ "$remove" = "YES" -a "$atype" != "none" ]
then 
	delete $fname
	/usr/lib/lpforms -f $fname -x 1> /dev/null 2>/dev/null

      if [ "$atype" = "mail" ] ; then
	/usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -u allow:"$users" -A "mail  $LOGNAME" -W $freq -Q $npreq > /dev/null
      else
	/usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -u allow:"$users" -A "$atype" -W $freq -Q $npreq > /dev/null
      fi

elif [ "$remove" = "YES" -a "$atype" = "none" ]
then 
	delete $fname
	/usr/lib/lpforms -f $fname -x 1> /dev/null 2>/dev/null
	/usr/lib/lpforms -f $fname -F /usr/tmp/uform.$VPID -uallow:"$users" -A none > /dev/null
fi


rm -rf /usr/tmp/uform.$VPID /usr/tmp/chgform.$VPID 

echo 0
