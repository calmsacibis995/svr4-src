#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkregdisp.sh	1.2.3.1"
# Display values written to temp file as they appear on the bkreg/add task

tfile=`bkregvals -t $2 $1`
if [ $? -ne 0 ]
then
	echo Unable to retrieve field values for entry.
	rm -f $tfile
	exit 1
fi


echo 
echo Originating name: `grep oname $tfile | sed -e "s/oname=//"`
echo Originating device: `grep odevice $tfile | sed -e "s/odevice=//"`
echo Originating media name: `grep olabel $tfile | sed -e "s/olabel=//"`
echo 
echo Weeks: `grep week $tfile | sed -e "s/week=//"`
echo Days: `grep day $tfile | sed -e "s/day=//"`
echo 
echo Method: `grep method $tfile | sed -e "s/method=//"`
echo Backup options: `grep options $tfile | sed -e "s/options=//"`
echo 
echo Dependencies: `grep depend $tfile | sed -e "s/depend=//"`
echo 
echo Destination group: `grep dgroup $tfile | sed -e "s/dgroup=//"`
echo Destination device: `grep ddevice $tfile | sed -e "s/ddevice=//"`
echo Destination characteristics:
echo `grep dchar $tfile | sed -e "s/dchar=//"`
echo Destination media names:
echo `grep dmname $tfile | sed -e "s/dmname=//"`

rm -f $tfile
exit 0
