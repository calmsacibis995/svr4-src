#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)dfs.cmds:unshareall/unshareall.sh	1.3.3.1"
# unshareall  -- unshare resources

USAGE="unshareall [-F fsys[,fsys...]]"
fsys=
set -- `getopt F: $*`
if [ $? != 0 ]		# invalid options
	then
	echo $USAGE >&2
	exit 1
fi
for i in $*		# pick up the options
do
	case $i in
	-F)  fsys=$2; shift 2;;
	--)  shift; break;;
	esac
done

if [ $# -gt 0 ]		# accept no arguments
then
	echo $USAGE >&2
	exit 1
fi

if [ "$fsys" ]		# for each file system ...
then
	fsys=`echo $fsys|tr ',' ' '`
else			# for every file system ...
	fsys=`sed 's/^\([^# 	]*\).*/\1/' /etc/dfs/fstypes`
fi

for i in $fsys
do
	for path in `sed -n "s/^\([^ 	]*\)[ 	]*[^ 	]*[ 	]*${i}.*/\1/p" /etc/dfs/sharetab`
	do
		/usr/sbin/unshare -F $i $path
	done
done
