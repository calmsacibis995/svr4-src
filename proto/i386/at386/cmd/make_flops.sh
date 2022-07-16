#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

:


#ident	"@(#)proto:i386/at386/cmd/make_flops.sh	1.3.2.1"

PROTO_DIR=$1

VER=4.0
DRIVE_INFO=${PROTO_DIR}/drive_info
FDRIVE=`cut -f1 ${DRIVE_INFO}`
BLKCYLS=`cut -f2 ${DRIVE_INFO}`

cd $ROOT
lab=`cat ${PROTO_DIR}/LABEL | sed -n "1p"`
fcnt=`expr "$lab" : "$VER.3 386unix Fnd Set 1 of \([0-9]*\)"`
i=`expr 3`
while [ $i -le $fcnt ]
do
  echo "Insert FLOPPY #$i of $fcnt and then press <RETURN>, s to skip or F not to format> \c"
  read x
  if [ "$x" = "s" ]
	then i=`expr $i + 1`
	continue
  fi
  if [ "$x" != "F" ]
  then
	format -i 2 /dev/rdsk/${FDRIVE}t
  fi
  l_seq=`echo $i | awk '{printf "%.2d",$1}'`
  echo "$VER.3 386unix Fnd Set $i of $fcnt" |
	dd of=/dev/rdsk/${FDRIVE}t bs=512 count=1 seek=`expr ${BLKCYLS:-30} - 1`
  cpio -ocB -O /dev/rdsk/$FDRIVE < ${PROTO_DIR}/FLOPPY/cpiolist${l_seq}
  i=`expr $i + 1`
done
