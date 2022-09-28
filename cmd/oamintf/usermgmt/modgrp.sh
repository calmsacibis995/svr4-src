#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/modgrp.sh	1.3.3.1"

################################################################################
#	Command Name: modgrp
#
#	Description: This scripts does 3 things: 1) modifies group information
#		     2) changes primary group for specified logins 3) adds
#		     supplementary group status to specified logins.
#
# 	Inputs:		$1 - Group name
#			$2 - New Group name
# 			$3 - group ID
# 			$4 - primary group
# 			$5 - supplementary groups
################################################################################

# There are two pieces of information about the group entry that can
#   change: name and ID.  Since both have been validated and tested for
#   uniqueness prior to calling this command, we can use the override
#   (-o) option when changing the group info.  However, we do have to test
#   if the name has changed or not.

GNAME="${1}"
N_GNAME="${2}"
GID="${3}"
PRIGRP="${4}"
SUPGRP="${5}"
	
if [ $# -ne 5 ]
then
	echo "Usage: ${0}: grp_name new_grp_name gid pri_grp sup_grp"
fi

if [ $2 = $1 ]
then
	groupmod -g ${GID} -o ${GNAME}
else
	groupmod -g ${GID} -o -n ${N_GNAME} ${GNAME}
fi

# change primary group for specified logins
if [ "${PRIGRP}" != "NULL" ]
then
	for x in `echo ${PRIGRP} | sed 's/,/ /g'`
	do
		usermod -g "${GNAME}" "$x" || exit 1
	done
fi

# change supplementary group members
if [ "${SUPGRP}" != "NULL" -o "${SUPGRP}" != "" ]
then
	addgrpmem -g ${N_GNAME} `echo ${SUPGRP} | sed 's/,/ /g'`
fi
