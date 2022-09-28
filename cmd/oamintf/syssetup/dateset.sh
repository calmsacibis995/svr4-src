#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:syssetup/dateset.sh	1.2.3.2"
################################################################################
#
#	Module Name: dateset
#
#
#	Calling Sequence: This script is invoked through the Form.datetime
#			  form in "syssetup" Menu or from the Setup Task.
#
#	Functional Description:	Calls date to set mm/dd/yy and hh:mm and
#                               puts TZ into /etc/TIMEZONE for Form.datetime.
#
#	Inputs: $1-$8 are MM DD YY hh mm am/pm TZ DST.
#
#	Outputs: Exit code returned to Form.datetime for Text file selection.
#                If return code is true (okay) then cleanup is performed,
#                Otherwise, sent back to Form.datetime to try again.
#
#	Functions Called: date and /usr/sbin/cron
#
#	Issues:
#		1) 
#		  
###############################################################################

if test $# -ne 8 
then	echo '   datetime: Not enough information.' >>/tmp/admerr 2>&1
	exit 1
fi

tz=
tc=
dst=
timechanged=

case "${7}" in
'Greenwich' | 'GMT' | 'GMT0' | 'GMT0GDT')	tz="GMT0" ;;
 'Atlantic' | 'AST' | 'ADT'  | 'AST4' | 'AST4AMT') 	tz="AST4" ;;
  'Eastern' | 'EST' | 'EDT'  | 'EST5' | 'EST5EDT') 	tz="EST5" ;;
  'Central' | 'CST' | 'CDT'  | 'CST6' | 'CST6CDT') 	tz="CST6" ;;
 'Mountain' | 'MST' | 'MDT'  | 'MST7' | 'MST7MDT') 	tz="MST7" ;;
  'Pacific' | 'PST' | 'PDT'  | 'PST8' | 'PST8PDT') 	tz="PST8" ;;
    'Yukon' | 'YST' | 'YDT'  | 'YST8' | 'YST8YDT') 	tz="YST8" ;;
   'Alaska' | 'AST10' | 'AST10ADT') 	tz="AST10" ;;
   'Bering' | 'BST' | 'BDT'  | 'BST11' | 'BST11BDT') 	tz="BST11" ;;
   'Hawaii' | 'HST' | 'HST10' | 'HST10HDT') 	tz="HST10" ;;
       		             *)		tz="EST5" ;;
esac
tc=`echo $tz | cut -c1-3`
if test "$8" = "yes"
then
	dst=`expr "${tc}" : '\(.\)'`DT
fi

TZ=${tz}${dst}

rootid=`id | cut -f2 -d"=" | cut -f1 -d"("`
if test "$rootid" = "0"
then
	echo "# established `date`\nTZ=${TZ}\nexport TZ" >/etc/TIMEZONE ||
	{
		echo '   datetime:  Time zone has not been changed.  Cannot write /etc/TIMEZONE' >>/tmp/admerr 2>&1
		exit 1
	}
else
	echo "   datetime: Timezone has not been changed." >> /tmp/admerr 2>&1
	echo "   Must be root uid to write /etc/TIMEZONE." >> /tmp/admerr 2>&1
	exit 1
fi
export TZ

#Note: Any logins and processes running when the time zone changes, and 
#all their child processes, will continue to see the old time zone.
#The cron(1M) will be restarted at the end of this procedure.'
timechanged=yes

case "${1}" in
	'Jan' | 'January')	mm="01";;
	'Feb' | 'February')	mm="02";;
	'Mar' | 'March')	mm="03";;
	'Apr' | 'April')	mm="04";;
	'May' | 'May')		mm="05";;
	'Jun' | 'June')		mm="06";;
	'Jul' | 'July')		mm="07";;
	'Aug' | 'August')	mm="08";;
	'Sep' | 'September')	mm="09";;
	'Oct' | 'October')	mm="10";;
	'Nov' | 'November')	mm="11";;
	'Dec' | 'December')	mm="12";;
	'*')			mm="";;
esac

if test `expr $2 : '.*'` = 1
	then 	dd="0${2}"
	else	dd=${2}
fi
if test `expr $3 : '.*'` = 4
	then yy=`echo ${3} | cut -c3-4`
	else yy=${3}
fi
if test `expr $4 : '.*'` = 1
	then 	Hh="0${4}"
	else	Hh=${4}
fi
if test "${6}" = "pm" || test "${6}" = "PM"
then
	case "${Hh}" in
	'01')	HH="13" ;;
	'02')	HH="14" ;;
	'03')	HH="15" ;;
	'04')	HH="16" ;;
	'05')	HH="17" ;;
	'06')	HH="18" ;;
	'07')	HH="19" ;;
	'08')	HH="20" ;;
	'09')	HH="21" ;;
	'10')	HH="22" ;;
	'11')	HH="23" ;;
	'12')	HH="12" ;;
	'*')	HH="" ;;
	esac
else	
	if test "${Hh}" = "12"
	then
		HH="00"
	else	HH=${Hh}
	fi
fi
if test `expr $5 : '.*'` = 1
	then 	MM="0${5}"
	else	MM=${5}
fi


date $mm$dd$HH$MM$yy >>/tmp/checkdate 2>&1 ||
{
	echo '   datetime: Unable to set date.' >>/tmp/admerr 2>&1
	exit 1
}
echo "   The date and time are now changed." >>/tmp/checkdate
timechanged=yes


if [ ${timechanged} ]
then
	pid=`ps -e | sed -n '/ cron$/s/^ *\([0-9]\{1,\}\) .*/\1/p'`
	if [ "${pid}" ] &&  kill ${pid} >>/tmp/admerr 2>&1 
	then
		sleep 2
		cron >>/tmp/checkdate 2>&1 || {
			echo '   datetime: Cannot restart cron.' >>/tmp/admerr 2>&1
			exit 1
			}
		echo '   The cron has been restarted to pick up' >>/tmp/checkdate
		echo '   the new time and/or timezone.' >>/tmp/checkdate
	fi
fi
