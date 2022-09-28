#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/bin/doexec.sh	1.1.1.1"

#	doexec
#	Options:
#	d - display preference
#	D - command is an MS-DOS command without the leading "dos -c"
#	n - command string takes no arguments

USAGE="usage: doexec [-d<display preference>] [-D] [-n] \"command string\""
cmd_type=U
param=y
display=x,vt,tty
cmd_arg=

if type getopts | grep 'not found' > /dev/null
then
	set -- `getopt d:Dn $*`
	if [ $? != 0 ]
	then	echo "$USAGE"
		exit 2
	fi

	for i in $*
	do
		case $i in
		-D)	if [ -f /usr/vpix/dosinst ]
			then	cmd_type=D
				shift
			else	message -d "doexec: Error attempting to use \"-D\" option.
        MS-DOS operating system not installed."
				exit 1
			fi
			;;
		-d)	dflg="$2"; shift 2;;
		-n)	param=n;;
		--)	shift; break;;
		esac
	done
else
	while getopts d:Dn i
	do
		case $i in
		d)	display="$OPTARG";;
		D)	if [ -f /usr/vpix/dosinst ]
			then	cmd_type=D
			else	message -d "doexec: Error attempting to use \"-D\" option.
        MS-DOS operating system not installed."
				exit 1
			fi
			;;
		n)	param=n;;
		\?) 	echo "$USAGE"
			exit 0
			;;
		esac
	done
	shift `expr $OPTIND - 1`
fi

cmd=`/usr/vmsys/bin/slash "$1"`

if [ "$cmd_type" = D ]
then	:
else
	eval /usr/vmsys/admin/swset/cmdck "$cmd" > /dev/null
	if [ $? != 0 ]
	then	message -d "doexec: Error: \"$cmd\" not found.
        Cannot proceed."
		exit 1
	fi
fi

if [ "$param" = y ]
then	echo "Enter parameter(s) for \"$cmd\": \c"
	cmd_arg=`/usr/vmsys/bin/getword`
fi

if [ ! -x /usr/bin/displayenv ]
then	env=tty
else	env=`/usr/bin/displayenv`
	case "$env" in
	tty | vt | xvt | x)	;;
	*)			env=tty;;
	esac
fi

display=`echo "$display,tty" | tr , ' '`
set -- "$display"
if [ "$cmd_type" = D ]
then
	for pref in $*
	do
	case $pref in
	tty)	case $env in
		tty | vt)	dos -c "$cmd $cmd_arg";;
		xvt | x)	dos -t -c "$cmd $cmd_arg";;
		esac
		;;
	vt)	case $env in
		tty | x)	continue;;
		vt)		dos -c "$cmd $cmd_arg&";;
		xvt)		newvt -e "dos -c \"$cmd $cmdarg\"";;
		esac
		;;
	x)	case $env in
		tty | vt)	continue;;
		xvt | x)	dos -c "$cmd $cmdarg&";;
		esac
		;;
	*)	message -d "Error: illegal display preference specified."
		exit 1
		;;
	esac
	break
	tput init
	done
else
	for pref in $*
	do
	case $pref in
	tty)	eval "$cmd $cmd_arg";;
	vt)	case $env in
		tty | x)	continue;;
		xvt | vt)	newvt -e "$cmd $cmd_arg;echo \"\\nStrike the ENTER key to continue\\c\";read a";;
		esac
		;;
	x)	case $env in
		tty | vt)	continue;;
		xvt | x)	/usr/X/bin/xterm -e sh -c "$cmd $cmd_arg;echo \"\nStrike the ENTER key to continue\\c\";read a"&;;
		esac
		;;
	*)	message -d "Error: illegal display preference specified."
		exit 1
		;;
	esac
	break
	done
fi
