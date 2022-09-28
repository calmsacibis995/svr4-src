#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)portmgmt:port_quick/q-rm.sh	1.1.3.3"

# PURPOSE: Remove currently configured RS232 port
# ---------------------------------------------------------------------
STREAM="NULL"
REBOOT="NO"

cp /etc/conf/sdevice.d/asy /usr/tmp/Osdevice.d
cp /etc/conf/node.d/asy /usr/tmp/Onode.d
cp /etc/inittab /usr/tmp/Oinittab
cp /etc/saf/ttymon3/_pmtab /usr/tmp/pmtab

for i in $*
do
TTY=`echo $i | cut -c11-16`

if [ $TTY = 01 -o $TTY = 01s -o $TTY = 01h ]
then
	grep -v "$TTY" /etc/conf/node.d/asy > /usr/tmp/node$$
	cp /usr/tmp/node$$ /etc/conf/node.d/asy
	rm -f /usr/tmp/node$$ 
	REBOOT=YES
	fi

grep "01h" /etc/conf/node.d/asy
RETURNh=$?
grep "01s" /etc/conf/node.d/asy 
RETURNs=$?
if [ $RETURNs != 0  -a $RETURNh != 0 ]
then
	grep -v "01" /etc/conf/node.d/asy > /usr/tmp/node$$
	cp /usr/tmp/node$$ /etc/conf/node.d/asy
	grep -v "asy	Y	1	7	1	3	2f8	2ff	0	0" /etc/conf/sdevice.d/asy > /usr/tmp/sd$$
	cp /usr/tmp/sd$$ /etc/conf/sdevice.d/asy
	rm -f /usr/tmp/sd$$ /usr/tmp/node$$
	REBOOT=YES
fi

if [ $TTY = 00 -o $TTY = 00s -o $TTY = 00h ]
then
	grep -v "$TTY" /etc/conf/node.d/asy > /usr/tmp/node$$
	cp /usr/tmp/node$$ /etc/conf/node.d/asy
	rm -f /usr/tmp/node$$
	REBOOT=YES
	fi

# See if the device specifies is
# a streams or a clist based device.
# If it is clist then a getty is removed in inittab
# If it is not, then sacadm is used for ttymon.

# The isastream program will return a 1 for a stream device
# a 0 for a clist based device.

if [ $TTY = 01 -o $TTY = 01s $ -o $TTY = 01h ]
then
	STREAM=YES
else
/usr/sadm/sysadm/bin/isastream /dev/term/$TTY
RET=$?
if [ $RET = 1 ]
then
	STREAM=YES
else
	if [ $RET = 0 ]
	then
		STREAM=NO
	else
		echo "Cannot open port $i.\n " >>/usr/tmp/ap.$VPID
		continue
	fi
fi
fi

if [ "$STREAM" = "NO" ]
then

	trap '' 1 2 3 9 15

	DOTTTY=$TTY

	rm -f /etc/conf/init.d/ua_${TTY} /etc/conf/rc.d/boot_${TTY} 

	if [ -c /dev/${DOTTTY} ]
	then
		chmod 666 /dev/${DOTTTY}
		chown root /dev/${DOTTTY}
		chgrp sys /dev/${DOTTTY}
	fi
	trap '' 1 2 3 9 15
	/etc/conf/bin/idmkinit -o /usr/tmp
	cp /usr/tmp/inittab /usr/tmp/inittab$$
	chown bin /usr/tmp/inittab$$
	chgrp bin /usr/tmp/inittab$$
	chmod 444 /usr/tmp/inittab$$
	cp /usr/tmp/inittab$$ /etc/inittab
	telinit q
	
echo "$i was successfully removed.\n" >>/usr/tmp/ap.$VPID:1

else

	pmadm -r -p ttymon3 -s $TTY >/dev/null 2>&1
	echo "$i was successfully removed.\n" >>/usr/tmp/ap.$VPID
fi
done


if [ $REBOOT = YES ]
then
	rm -f /usr/tmp/Osdevice.d /usr/tmp/Onode.d /usr/tmp/pmtab
	rm -f /usr/tmp/inittab$$ /usr/tmp/inittab /usr/tmp/Oinittab
	/etc/conf/bin/idmknod > /dev/null 2>&1
fi


echo 0
exit 0
