#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/uiddflt.sh	1.2.3.1"

################################################################################
#	Command Name: uiddflt
#
# 	This functions is used for finds the default uid for Form.addusr.
################################################################################

# Sort 3rd field in /etc/passwd then cut 3rd field
# from last line.
defuid=`sort -u -t: +2n /etc/passwd | cut -d: -f3`

# Assign maxresid the default maximum reserved user ID
maxresid=`getusrdefs -r | cut -d= -f2`

# Assign minid the maximum reserve user ID + 1.  This is the minimum
# user ID to be assigned.
minid=`expr $maxresid + 1`

# eliminate all userid's less than maximum reserve user ID, then find first available
# user ID greater than minid.
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
