#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/chk_logins.sh	1.3.3.1"

################################################################################
#	Commad Name: chk_logins
#
# 	This script is used for validating that exist.  Used in
#	Form.addgrp and Form.modgrp2
################################################################################

test -z "$1" && exit 0	# Optional field, can be blank

sed -e 's/^\([^:]*\):[^:]*:\([^:]*\).*/\1,\2/p' \
-e '/,[0-9]\{1,2\}$/d' /etc/passwd > /tmp/$$.logins

# verify that groups entered are valid
for x in `echo $1 | sed 's/,/ /g'`
do

	if grep "^$x," /tmp/$$.logins > /dev/null ||
	   grep ",$x$" /tmp/$$.logins > /dev/null
	then
		 continue
	else
		 rm -f /tmp/$$.logins
		 echo $x > /tmp/ln
		 exit 1
	fi
	
done

rm -f /tmp/$$.logins
exit 0
