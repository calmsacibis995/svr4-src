#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:interface/itemhelp.sh	1.1.3.1"

SPSET="[:]"
NSPSET="[^:]"

tsedit() {
	sed -n "
		/^#/d;
		/^$1$SPSET/,/^[^	 ]/ {
			s/^$1$SPSET[	 ]*\([^	 ]\{1,\}.*\)/\1/p
		}
	" $2
}

sedit() {
	sed -n "
		/^#/d;
		/^$1$SPSET/,/^[^	 ]/ {
			s/^	/   /p
		}
	" $2
}

mesg() {
	echo ""
	sedit "$1" $2
	echo ""
}

if [ "x$1" = x-l ]
then
	rows=`mesg "$2" $3 | wc -l`
	[ "$rows" -gt 12 ] &&
		rows=12
	echo $rows
elif [ "x$1" = x-t ]
then
	FRM=`expr $2 : "\(.*$SPSET\)$NSPSET*"`
	TITLE=`tsedit "$2" $3`
	if [ -z "$TITLE" ]
	then
		TITLE=`tsedit "${FRM}TITLE" $3`
		[ -z "$TITLE" ] &&
			TITLE=`tsedit "TITLE" $3`
	fi
	[ -z "$TITLE" ] &&
		TITLE="HELP"
	echo "Help on $TITLE"
elif [ "x$1" = x-g ]
then
        grep "$2:ABSTRACT" $3 > /dev/null
        if [ $? -eq 0 ]
        then
                rows=`mesg "$2:ABSTRACT" $3 | wc -l`
        else
                rows=`mesg "ABSTRACT" $3 | wc -l`
        fi
        [ "$rows" -gt 12 ] &&
                rows=12
        echo $rows
elif [ "x$1" = x-a ]
then
	grep "$2:ABSTRACT" $3 > /dev/null
	if [ $? -eq 0 ]
	then
		mesg "$2:ABSTRACT" $3
	else
		mesg "ABSTRACT" $3
	fi
else
	mesg "$1" $2
fi
