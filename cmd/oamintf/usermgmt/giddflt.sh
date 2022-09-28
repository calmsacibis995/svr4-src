#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/giddflt.sh	1.2.3.1"

################################################################################
#	Command Name: giddflt
#
# 	This functions is used for finding the next available Group ID 
################################################################################

# Sort 3rd field in /etc/group then cut 3rd field
# from last line.

defuid=`sort -u -t: +2n /etc/group | cut -d: -f3`

# assign minid the default reseve id

minid=`getusrdefs -r | cut -d= -f2`

# eliminate all userid's less than minid, then find first available
# userid number greater than minid.

for n in `echo ${defuid} | sed -e 's% \([1-9][0-9][0-9][0-9]*\)%_\1%;s%.*_%%'`
do
if [ $minid -ne $n ]
then
	break
else
	minid=`expr $minid + 1`
fi
done
echo $minid
