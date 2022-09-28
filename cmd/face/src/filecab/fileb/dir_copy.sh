#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/dir_copy.sh	1.1"

# usage: dir_copy source destination
# no need to redirect stderr in this file, spawnv
set -e
pwd=`pwd`
if [ -n "$3" ]
then
	path="$2/$3"
else
	path="$2/`basename $1`"
fi
mkdir "$path" 2>/dev/null

# make sure path is absolute so we can get to it from "$1" directory

if [ `expr "$path" : '/.*' || :` = 0 ]
then
	path="$pwd/$path"
fi
cd "$1"
find . -print | cpio -pmud "$path"
