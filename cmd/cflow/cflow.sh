#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# ident	"@(#)cflow:cflow.sh	1.9.1.9"
USRLIB=${USRLIB:-/usr/ccs/lib}		# where the executables are
INVFLG=					# invert caller:callee relationship
DFLAG=					# depth for flowgraph
AFLAG=					# report duplicate calls
IFLAG=					# ix: include external, static:
					#      ix doesn't work right
					# i_: include names with _
NDOTC=0					# how many files to process
CONLY=					# first pass only
LINTF=					# options to pass to lint
TMPDIR=${TMPDIR:-/usr/tmp}		# place to put temp files
TMP=$TMPDIR/cf.$$			# temp file for first pass
TMPG=$TMPDIR/tcf.$$			# temp file for second pass

#
# If we are in a cross environment, pick up the correct nm and lint
# (they will be prefixed with PFX)
#
NM=${PFX}nm
LINT=${PFX}lint
AS=${PFX}as

#
# Abbreviations for the 4 cflow programs.
#
LPFX=$USRLIB/lpfx
FLIP=$USRLIB/flip
DAG=$USRLIB/dag
NMF=$USRLIB/nmf

trap "rm -f $TMP $TMPG; kill $$" 1 2 3
USAGE="Usage: cflow [-acrV] [-ix] [-i_] [-d tag] files ..."
BADUSE="cflow: file with unknown suffix ignored:"
ERRORS="cflow: errors in source file"
OPTARG=Oagrcd:i:I:D:U:Y:V

set -- `getopt $OPTARG "$@"`
if [ $? -ne 0 ]
then
    echo $USAGE >&2
    rm -f $TMP $TMPG
    exit 2
fi

while [ $# -gt 0 ]
do
    case $1 in
	-[Og])	shift;;
	-V)	$LPFX -V; shift;;
	-r)	INVFLG=1; shift;;
	-c)	CONLY=1; shift;;
	-a)	AFLAG="$1"; shift;;
	-d)	DFLAG="$1$2"; shift 2;;
	-i)	IFLAG="$IFLAG $1 $2"
		if [ "$2" = "x" ]
		then
			LINTF="$LINTF -x"
		fi
		shift 2;;
	-[IDUY])	LINTF="$LINTF $1$2"; shift 2;;
	--)	shift;
		while [ $# -gt 0 ]
		do
		    case $1 in	
			-*)	set -- `getopt $OPTARG "$@"`;
				if [ $? -ne 0 ]
				then
				    echo $USAGE >&2
				    exit 2
				fi
				break;;
			*.[cilyos])	
				FILES="$FILES $1"
				NDOTC=`expr $NDOTC + 1`
				shift;;
			*.cf)	FILES="$FILES $1"; shift;;
			*)	echo $BADUSE $1 >&2
				shift;;
		    esac
		done
		;;
	-*)	echo "cflow: bad option ignored: $1" >&2; shift;;
    esac
done

#
# Give something to cflow to do!
#
if [ "$FILES" = "" ]
then
	echo "cflow: no file arguments" >&2
	rm -f $TMP $TMPG
	exit 1
fi

#
# Process the files
#
for I in $FILES
do
	case $I in
	#
	# Run pass2 only on a .cf file
	#
	*.cf)
		if [ "$CONLY" = "" ]
		then
			cat $I >> $TMPG
		fi
		;;

	#
	# .y and .l files should be passed directly to cflow, rather than
	# the file proceduced by yacc/lex; otherwise line numbers
	# will get confused.
	#
	*.[yl])
		case $I in
		*.y) CMD=yacc; SUF=y; CMDFILE=y.tab.c;;
		*.l) CMD=lex; SUF=l; CMDFILE=lex.yy.c;;
		esac
		$CMD $I
		if [ $? != 0 ]
		then
			echo $ERRORS >&2
			rm -f $TMP $TMPG $CMDFILE
			exit 1
		fi
		sed -e "/^# line/d" $CMDFILE > $I.c
		$LINT $LINTF -W $TMP $I.c > /dev/null
		if [ $? != 0 ]
		then
			echo $ERRORS >&2
			rm -f $TMP $TMPG $CMDFILE $I.c
			exit 1
		fi
		if [ "$CONLY" ]
		then
			$LPFX $IFLAG < $TMP > `basename $I .$SUF`.cf
		else
			$LPFX $IFLAG < $TMP >> $TMPG
		fi
		rm $CMDFILE $I.c
		;;

	*.[ci])
		case $I in
		*.c)  SUF=c;;
		*.i)  SUF=i;;
		esac
		$LINT $LINTF -W $TMP $I > /dev/null
		if [ $? != 0 ]
		then
			echo $ERRORS >&2
			rm -f $TMP $TMPG
			exit 1
		fi
		if [ "$CONLY" ]
		then
			$LPFX $IFLAG < $TMP > `basename $I .$SUF`.cf
		else
			$LPFX $IFLAG < $TMP >> $TMPG
		fi
		;;

	*.[os])
		TMPNM=$TMPDIR/cfnm.$$
		case $I in
		*.s) SUF=s; $AS -o $TMP.o $I
			if [ $? != 0 ]
			then
				echo $ERRORS >&2
				rm -f $TMP $TMPG
				exit 1
			fi
			DOTO=$TMP.o;;
		*.o) SUF=o; DOTO=$I;;
		esac

		a=`basename $I .$SUF`
                $NM -he $DOTO > $TMPNM 
		if [ $? != 0 ]
		then
			echo $ERRORS >&2
			rm -f $TMP $TMPG $TMPNM
			exit 1
		fi
		if [ "$CONLY" ]
		then
			$NMF $a ${a}.$SUF < $TMPNM > $a.cf
		else
			$NMF $a ${a}.$SUF < $TMPNM >>$TMPG
		fi
		rm -f $TMPNM
		;;
	esac
done

if [ "$CONLY" = "" ]
then
	if [ "$INVFLG" != "" ]
	then
		sed -n "/=/p" < $TMPG > $TMP.q
		sed -n "/:/p" < $TMPG | $FLIP >> $TMP.q
		sort < $TMP.q > $TMPG
		rm $TMP.q
	fi
	$DAG $DFLAG $AFLAG < $TMPG
	rm -f $TMP.?
fi

rm -f $TMP $TMPG
exit 0
