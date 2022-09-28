#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)spell:spell.sh	1.12"
#	spell program
# B_SPELL flags, D_SPELL dictionary, F_SPELL input files, H_SPELL history, S_SPELL stop, V_SPELL data for -v
# L_SPELL sed script, I_SPELL -i option to deroff
H_SPELL=${H_SPELL-/var/adm/spellhist}
V_SPELL=/dev/null
F_SPELL=
B_SPELL=
L_SPELL="sed -e \"/^[.'].*[.'][ 	]*nx[ 	]*\/usr\/lib/d\" -e \"/^[.'].*[.'][ 	]*so[ 	]*\/usr\/lib/d\" -e \"/^[.'][ 	]*so[ 	]*\/usr\/lib/d\" -e \"/^[.'][ 	]*nx[ 	]*\/usr\/lib/d\" "
USAGE="usage: ${0} [-v] [-b] [-x] [-l] [-i] [+local_file] [ files ]"
trap "rm -f /tmp/spell.$$; exit" 0 1 2 13 15
for A in $*
do
	case $A in
	-v)	if /bin/pdp11
			then	echo -v option not supported on pdp11 >&2
				EXIT_SPELL=exit
		else	B_SPELL="$B_SPELL -v"
			V_SPELL=/tmp/spell.$$
		fi ;;
	-a)	: ;;
	-b) 	D_SPELL=${D_SPELL-/usr/share/lib/spell/hlistb}
		B_SPELL="$B_SPELL -b" ;;
	-x)	B_SPELL="$B_SPELL -x" ;;
	-l)	L_SPELL="cat" ;;
	+*)	if [ "$FIRSTPLUS" = "+" ]
			then	echo "multiple + options in spell, all but the last are ignored" >&2
		fi;
		FIRSTPLUS="$FIRSTPLUS"+
		if  LOCAL=`expr $A : '+\(.*\)' 2>/dev/null`;
		then if test ! -r $LOCAL;
			then echo "spell cannot read $LOCAL" >&2; EXIT_SPELL=exit;
		     fi
		else echo "spell cannot identify local spell file" >&2; EXIT_SPELL=exit;
		fi ;;
	-i)	I_SPELL="-i" ;;
	-*)	echo $USAGE
		exit;;
	*)	F_SPELL="$F_SPELL $A"
	esac
	done
${EXIT_SPELL-:}
cat $F_SPELL | eval $L_SPELL |\
 deroff -w $I_SPELL |\
 sort -u +0 |\
 /usr/lib/spell/spellprog ${S_SPELL-/usr/share/lib/spell/hstop} 1 |\
 /usr/lib/spell/spellprog ${D_SPELL-/usr/share/lib/spell/hlista} $V_SPELL $B_SPELL |\
 comm -23 - ${LOCAL-/dev/null} |\
 tee -a $H_SPELL
who am i >>$H_SPELL 2>/dev/null
case $V_SPELL in
/dev/null)	exit
esac
sed '/^\./d' $V_SPELL | sort -u +1f +0
