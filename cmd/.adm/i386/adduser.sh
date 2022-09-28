#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.




#ident	"@(#)adm:i386/adduser.sh	1.1.3.3"

# add a user to the system
# 
rm -f /usr/tmp/addcfm

# Do all validations first

loginid=$1       #  login ID
fullname=$2      #  Users full name
userid=$3        #  User ID
logdir=$4        #  Home directory

if [ $# -eq 5 ]
then
	sysadm=$5	 #  System admin privileges ( Yes or No )
else
	sysadm=No
fi

USAGE="USAGE: $0 <login id> <name> <user id> <home directory>"

if [ $# -lt 4 ]
then
	echo $USAGE
	exit 1
fi

#	check if the length of input is longer than 8

length=`echo $loginid | wc -c`
if [ "$length" -gt "9" ]
then 
	echo "Login name can not be longer than eight characters."
	exit 1
fi

#check for illegal character (s)

if echo "$loginid" | grep "[^0-9a-z]" > /dev/null
then 
	echo "Only numbers & lower case letters are permitted in login name." 
	exit 1
fi

#	check if login name already exists

grep "^$loginid:" /etc/passwd >/dev/null
rc=$?
if [ $rc -eq 0 ]
then
	echo "$loginid already exists on your system. Choose another login name." 
	exit 1
fi

# Verify Full name doesn't contain illegal characters

echo "$fullname" | grep "[^[0-9]a-z A-Z.]" > /dev/null
if [ "$?" -eq "0" ]
then 
	echo "The full name can only be letters, numbers, and periods." 
	exit 1
fi

#check for input string length

length=`echo $fullname | wc -c`
if [ "$length" -gt "26" -o "$length" -lt "2" ]
then 
	echo "The full name has to be 1 to 25 characters." 
	exit 1
fi

#	Check for illegal range

if [ "$userid" -gt 50000 -o "$userid" -lt 100 ]
	then 
	echo "User ID number out of legal range (100-50000)."
	exit 1
fi

#	Check if input userid exists already

if grep "^[^:]*:[^:]*:0*$userid:" /etc/passwd > /dev/null
then
	echo "User Id already used on system, please choose another."
	exit 1
fi

if test -f "$logdir" -o -d "$logdir"
then
	echo "$logdir directory already exists. Choose another HOME directory."
	exit 1
fi

echo "$logdir" | grep "[^0-9_/.a-zA-Z]" >/dev/null
rc=$?
if [ "$rc" = "0" ]
then
	echo "Directory name contains illegal characters." 
	exit 1
fi

dir=$logdir
while [ "$dir" != "/" -a "$dir" != "." ]
do
	path=`basename $dir`
	dir=`dirname $dir`
	length=`echo "$path" | wc -c`
	if [ "$length" -gt "15" ]
	then
		echo "More than 14 characters in directory name." 
		exit 1
	fi
done

#
#	Set default group id to 1. (for now)
#

groupid=1

#
#	Echo entry into /etc/passwd
#

/usr/bin/passmgmt -a -c "$fullname" -h $logdir -g $groupid -u $userid $loginid

#
#	Make home directory 
#
umask 002
mkdir ${logdir}
if [ $? -ne 0 ]
then
	echo "Creation of ${logdir} failed."
	/usr/bin/passmgmt -d $loginid
	exit 1
fi
chgrp ${groupid} ${logdir}
chown ${userid} ${logdir}

#	System administrative privileges

if [ "$sysadm" = "Yes" ]
then
	if test -r /etc/adm.lock
	then
		:
	else
		> /etc/adm.lock
		echo "$loginid base-adm" >> /etc/.useradm
		rm -f /etc/adm.lock
	fi
fi


#
#      Give user a default .profile if /etc/stdprofile exists.
#

if [ -r /etc/stdprofile ]
then
		cp /etc/stdprofile ${logdir}/.profile
		chmod 644 ${logdir}/.profile
		chown ${userid} ${logdir}/.profile
fi

echo "Enter a password for $loginid:"
/bin/passwd $loginid
