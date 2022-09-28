#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:cmd/sccsdiff.sh	6.5"
#	DESCRIPTION:
#		Execute bdiff(1) on two versions of a set of
#		SCCS files and optionally pipe through pr(1).
#		Optionally specify bdiff segmentation size.

trap "rm -f /tmp/get[abc]$$;exit 1" 0 1 2 3 15

if [ $# -lt 3 ]
then
	echo "Usage: sccsdiff -r<sid1> -r<sid2> [-p] [-s<num-arg>] sccsfile ..." 1>&2
	exit 1
fi

for i in $@
do
	case $i in

	-*)
		case $i in

		-r*)
			if [ ! "$sid1" ]
			then
				sid1=`echo $i | sed -e 's/^-r//'`
			elif [ ! "$sid2" ]
			then
				sid2=`echo $i | sed -e 's/^-r//'`
			fi
			;;
		-s*)
			num=`echo $i | sed -e 's/^-s//'`
			;;
		-p*)
			pipe=yes
			;;
		*)
			echo "$0: unknown argument: $i" 1>&2
			exit 1
			;;
		esac
		;;
	*s.*)
		files="$files $i"
		;;
	*)
		echo "$0: $i not an SCCS file" 1>&2
		;;
	esac
done

for i in $files
do
	if get -s -p -k -r$sid1 $i > /tmp/geta$$
	then
		if get -s -p -k -r$sid2 $i > /tmp/getb$$
		then
			bdiff /tmp/geta$$ /tmp/getb$$ $num > /tmp/getc$$
		fi
	fi
	if [ ! -s /tmp/getc$$ ]
	then
		if [ -f /tmp/getc$$ ]
		then
			echo "$i: No differences" > /tmp/getc$$
		else
			exit 1
		fi
	fi
	if [ "$pipe" ]
	then
		pr -h "$i: $sid1 vs. $sid2" /tmp/getc$$
	else
		cat /tmp/getc$$
	fi
done

trap 0
rm -f /tmp/get[abc]$$
