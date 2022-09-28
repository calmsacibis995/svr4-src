#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/grpchk.sh	1.1.3.1"

#grpchk
# for verifying group exists

(sed 's/$/:/
s/^\([^:]*\):[^:]*:\([^:]*\).*/\1,\2/p' /etc/group | sort
) >/tmp/lsgrp

grp=${1}
if grep "${grp}" /tmp/lsgrp >/dev/null 2>&1
then rm -f /tmp/lsgrp
     exit 0
else rm -f /tmp/lsgrp
     exit 1
fi

