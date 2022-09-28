#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/chk_sgrp.sh	1.2.3.1"

################################################################################
#	Command Name: chk_sgrp
#
# 	Description: This functions is used for validating the Supplementary
#		     Group field in Form.addusr.
#
#	Inputs:
#		$1 - Primary group entered in add user
#		$2 - Supplementary group(s) entered
################################################################################

OK=0
PRMGRP=1
BADGRP=2

test -z "$2" && exit $OK	# Optional field, can be blank


# Create a comma separated list of group names and ids.
sed -e 's/^\([^:]*\):[^:]*:\([^:]*\).*/\1,\2/p' \
-e '/,[0-9]\{1,2\}$/d' /etc/group > /tmp/$$.sgrp

GRPNAME=`grep "^$1," /tmp/$$.sgrp | cut -d, -f1`
test ! "$GRPNAME" && GRPNAME=`grep ",$1$" /tmp/$$.sgrp | cut -d, -f1`

GRPID=`grep "^$1," /tmp/$$.sgrp | cut -d, -f2`
test ! "$GRPID" && GRPID=`grep ",$1$" /tmp/$$.sgrp | cut -d, -f2`

# verify that groups entered are valid
for x in `echo $2 | sed 's/,/ /g'`
do
	if test "$x" = "$GRPNAME" || test "$x" = "$GRPID"
	then
		rm -f /tmp/$$.sgrp
 		exit $PRMGRP
	fi

	test "$x" = "other" || test "$x" = "1" && continue

	if grep "^$x," /tmp/$$.sgrp > /dev/null ||
	   grep ",$x$" /tmp/$$.sgrp > /dev/null
	then
		 continue
	else
		 rm -f /tmp/$$.sgrp
		 echo $x > /tmp/sgrp
		 exit $BADGRP
	fi
	
done

rm -f /tmp/$$.sgrp
exit $OK
