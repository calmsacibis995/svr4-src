#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cxref:cxref.sh	1.4.2.1"
USRLIB=${USRLIB:-/usr/ccs/lib}	# where the executables are
TMPDIR=${TMPDIR:-/usr/tmp}	# where to put temporary files
TOUT=$TMPDIR/tcx.$$		# combined input for second pass
LINTF=				# options to pass to lint
FILES=				# the *.c and *.cx files in order
NDOTC=0				# how many *.c were there
CONLY=				# set for "compile only" (no second pass)
LINK=				# set for link together all sources
SILENT=				# set with -s option
FULLPATH=	
OUTFILE=
FNUM=0
CXOPT=				# options to cxref
TMPFILES="$TMPDIR/tcx.$$ $TMPDIR/cx.$$.*"
XREF=$USRLIB/xref		# the cxref program
LINT=${PFX}lint			# lint (with prefix if in cross)

trap "rm -f $TMPFILES; exit 2" 1 2 3 15

USAGE="Usage: cxref [-scCVFdlt] [-o file] [-w width] [-L cols] [-Wname,file,func,line] files ..."
BADUSE="cxref: file with unknown suffix"
OPTARG=cVFD:Y:I:U:1:dlW:L:OgCw:tso:
set -- `getopt $OPTARG "$@"`
if [ $? -ne 0 ]
then
    echo "$USAGE" >&2
    rm -f $TMPFILES
    exit 2
fi
while [ $# -gt 0 ]
do
    case $1 in
	-[Og])	shift;;
	-C)	shift; CONLY=1;;
	-c)	shift; LINK=1;;
	-s)	shift; SILENT=1;;
	-F)	FULLPATH=1; LINTF="$LINTF $1"; shift;;
	-[IDUY])	LINTF="$LINTF $1$2"; shift 2;;
	-1)	LINTF="$LINTF $1$2"; shift 2;;
	-o)	OUTFILE=$2; shift 2;;
	-V)	$XREF -V; shift;;
	-[ldt])	CXOPT="$CXOPT $1"; shift;;
	-[LWw])	CXOPT="$CXOPT $1$2"; shift 2;;
	--)	shift;
		while [ $# -gt 0 ]
		do
		    case $1 in	
			-*)	set -- `getopt $OPTARG "$@"`;
				if [ $? -ne 0 ]
				then
				    echo "$USAGE" >&2
				    exit 2
				fi
				break;;
			*.[ci])	FILES="$FILES $1"
				NDOTC=`expr $NDOTC + 1`
				shift;;
			*.cx)	FILES="$FILES $1"; shift;;
			*)	echo $BADUSE $1 >&2
				exit 1
				shift;;
		    esac
		done
		;;
	-*)	echo "cxref: bad option ignored: $1" >&2;;
    esac
done

#
# No files specified!
#
if [ "$FILES" = "" ]
then
	echo "cxref: no file arguments" >&2
	exit 1
fi

#
# If no file has been specified for output, then direct to stdout
#
if [ "$OUTFILE" ]
then
	exec > $OUTFILE
fi

#
# Run pass1 (lint) only, creating .cx files.
# Any .cx files on the command line will be ignored.
#
if [ "$CONLY" ]
then
    for i in $FILES
    do
	case $i in
	*.cx)	;;
	*)	T=`basename $i .c`.cx
		if [ "$SILENT" = "" ]
		then
		    if [ "$FULLPATH" ]
		    then
			echo "$i:" 
		    else
			echo "`basename $i`:" 
		    fi
		fi
		$LINT $LINTF -R $TMPDIR/cx.$$ $i > /dev/null
		RETVAL=$?
		if [ $RETVAL = 0 ]
		then
		    mv $TMPDIR/cx.$$.lnt $T
		else
		    echo "cxref: errors in $i; no output created" >&2
		    rm -f $TMPFILES
		    exit $RETVAL
		fi;;
	esac
    done
else
    for i in $FILES
    do
	case $i in
	*.cx)	if [ "$SILENT" = "" ]
		then
		    if [ "$FULLPATH" ]
		    then 
			echo "$i:" 
		    else
			echo "`basename $i`:" 
		    fi
		fi
		if [ "$LINK" ]
		then
		    FNUM=`expr $FNUM + 1`
		    echo "M $FNUM" >> $TOUT
		    cat $i >> $TOUT
		else
		    $XREF $CXOPT < $i 
		    if [ $? -ne 0 ]
		    then
			echo "cxref: errors; no output created" >&2
			rm -f $TMPFILES
			exit 1
		    fi
		    echo " " 
		fi;;
	
	*)	if [ "$SILENT" = "" ]
		then
		    if [ "$FULLPATH" ]
		    then
			echo $i: 
		    else
			echo `basename $i`: 
		    fi
		fi
		$LINT $LINTF -R $TMPDIR/cx.$$ $i > /dev/null
		RETVAL=$?
		if [ $RETVAL = 0 ]
		then
		    if [ "$LINK" ]
		    then
			FNUM=`expr $FNUM + 1`
			echo "M $FNUM" >> $TOUT
			cat $TMPDIR/cx.$$.lnt >> $TOUT
		    else
			$XREF $CXOPT < $TMPDIR/cx.$$.lnt 
			if [ $? -ne 0 ]
			then
			    echo "cxref: errors; no output created" >&2
			    rm -f $TMPFILES
			    exit 1
			fi
			echo " " 
		    fi
		else
		    echo "cxref: errors in $i; no output created" >&2
		    rm -f $TMPFILES
		    exit $RETVAL
		fi
		;;
	esac
    done

    if [ "$LINK" ]
    then
	$XREF $CXOPT < $TOUT 
	if [ $? -ne 0 ]
	then
	    echo "cxref: errors; no output created" >&2
	    rm -f $TMPFILES
	    exit 1
	fi
    fi
    RETVAL=$?
fi

rm -f $TMPFILES
exit 0
