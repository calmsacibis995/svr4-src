#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)portmgmt:port_quick/q-add.sh	1.1.3.4"

# PURPOSE: Configure the software to a particular device type on RS232
#
#---------------------------------------------------------------------

rm -f /usr/tmp/ttylist* /usr/tmp/ap*

TTY00=`ls -l /dev/term/00s | cut -c1-45`
TTY01=`ls -l /dev/term/01s | cut -c1-45`
NOTTY00="NOTTY00"
NOTTY00h="NOTTY00h"
NOTTY01="NOTTY01"
NOTTY01h="NOTTY01h"

# lists ports configured and available for adding devices

if [ "$1" = "COLLECT" -o "$1" = "REMOVE" ]
then
	CONSOLE=`ls -l /dev/console | cut -c1-45`
	if [ "$CONSOLE" = "$TTY00" ]
	then
		NOTTY00=00s
		NOTTY00h=00h
	fi
	if [ "$CONSOLE" = "$TTY01" ]
	then
		NOTTY01=01s
		NOTTY01h=01h
	fi

	ls /dev/*vt00 /dev/term/???* | grep -v "$NOTTY00" | grep -v "$NOTTY00h" | grep -v "$NOTTY01"| grep -v "$NOTTY01h" >>/usr/tmp/ttylist.$VPID
fi

# See if $2 is tty01 we may have to build it into the kernel
cp /etc/conf/sdevice.d/asy /usr/tmp/Osdevice.d
cp /etc/conf/node.d/asy /usr/tmp/Onode.d
cp /etc/inittab /usr/tmp/Oinittab
STREAM=NULL
REBOOT=NO
MKNOD=NO
SPEED=$2
TYPE="$1"
PREFIX=tty
shift
shift
for i in $*
do
	TTY=`echo $i | cut -c11-14`

if [ $TTY = 00s -o $TTY = 00h ]
then
	grep "asy	Y	1	7	1	4	3f8	3ff	0	0" /etc/conf/sdevice.d/asy > /dev/null 2>&1
        if [ $? != 0 ]
	then
		echo "asy	Y	1	7	1	4	3f8	3ff	0	0" >> /etc/conf/sdevice.d/asy
		REBOOT=YES
	fi
		case $TTY in
		00s)
	
		grep 'asy	tty00	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty00	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		
		grep 'asy	term/00	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/00	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	tty00s	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty00s	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		
		grep 'asy	term/00s	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/00s	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		;;

		00h )
		grep 'asy	tty00	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty00	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		
		grep 'asy	term/00	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/00	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	tty00h	c	129' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty00h	c	129" >> /etc/conf/node.d/asy
			REBOOT=YES
		fi


		grep 'asy	term/00h	c	129' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/00h	c	129" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		;;
		esac
fi
if [ $TTY = 01s -o $TTY = 01h ]
then
	grep "asy	Y	1	7	1	3	2f8	2ff	0	0" /etc/conf/sdevice.d/asy > /dev/null 2>&1
        if [ $? != 0 ]
	then
	echo "asy	Y	1	7	1	3	2f8	2ff	0	0" >> /etc/conf/sdevice.d/asy
	fi	
		case $TTY in
		01s)
	
		grep 'asy	tty01	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty01	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	term/01	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/01	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	tty01s	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty01s	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi


		grep 'asy	term/01s	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo 'asy	term/01s	c	1' >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		;;

		01h)
		grep 'asy	tty01	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty01	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	term/01	c	1' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/01	c	1" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	tty01h	c	129' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	tty01h	c	129" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi

		grep 'asy	term/01h	c	129' /etc/conf/node.d/asy
                if [ $? != 0 ]
		then
			echo "asy	term/01h	c	129" >> /etc/conf/node.d/asy
			MKNOD=YES
		fi
		;;
		esac
fi
done

if [ $REBOOT = YES ]
then
	/etc/conf/bin/idbuild > /dev/null 2>&1
	if [ $? != 0 ]
	then	
		echo "An error was encountered while rebuilding the kernel. Configuration terminated. /dev/term/$TTY was not added." >>/usr/tmp/ap.$VPID
		# Errors encountered. Configuration terminated.
		cp /usr/tmp/Osdevice.d /etc/conf/sdevice.d/asy
		cp /usr/tmp/Onode.d /etc/conf/node.d/asy
		rm -f /usr/tmp/Osdevice.d /usr/tmp/Onode.d
	else
		# operation sucessful
		rm -f /usr/tmp/Osdevice.d /usr/tmp/Onode.d
		echo "In order to configure /dev/term/$TTY you must shutdown and reboot your system. Please shutdown your system as soon as possible." >>/usr/tmp/ap.$VPID
	fi
fi

	if [ $MKNOD = YES ]
	then
		# operation sucessful
		rm -f /usr/tmp/Osdevice.d /usr/tmp/Onode.d
		/etc/conf/bin/idmknod -s > /dev/null 2>&1
	fi

# We have four different port types to deal with.
# In all cases the ID will reflect the software device
# This means that a user will never be able to have
# a /etc/conf/init.d/ua_File for hardware and software.
# Also the file name MUST be unique.

for i in $*
do
TTY=`echo $i | cut -c11-14`
SFOZ=`echo $i | cut -c6`
FOSSTATA=`echo $i | cut -c9,12`
FOSSTATB=`echo $i | cut -c8,11`
S802=`echo $i | cut -c9`
SINT=`echo $i | cut -c11`

if [ $S802 != s ]
then
	if [ $SINT != s ]
	then
		if [ $FOSSTATA = vt00 -o $FOSSTATB = vt00 ]
		then
			ID=`echo $i | cut -c7-12`
			TID=F`echo $i | cut -c7-9`
			PREFIX=""
		else	if [ $SFOZ = s ]
			then
		        	LENGTH=`echo $i | cut -c8`
				if [ $LENGTH = t ]
				then
					ID=`echo $i | cut -c7-15`
					TID=`echo $i | cut -c5,11-12`
				else	ID=`echo $i | cut -c7-15`
					TID=`echo $i | cut -c7-8,11`
				fi
			else
				ID=`echo $i | cut -c9-15`
				TID=`echo $i | cut -c11-13`
			fi
		fi
	else
		ID=`echo $i | cut -c9-11`
		TID=`echo $i | cut -c9-11`
	fi
else
	ID=`echo $i | cut -c9-11`
	TID=`echo $i | cut -c9-11`
fi
    
# Now we will see if the device specified is
# a streams or a clist based device.
# If it is clist then a getty is spawned in inittab
# If it is not, then sacadm is used for ttymon.

# The isastream program will return a 1 for a stream device
# a 0 for a clist based device.

if [ $TTY = 00s -o $TTY = 00h -o $TTY = 01s -o $TTY = 01h ]
then
	STREAM=YES
else
#/usr/sadm/sysadm/bin/isastream /dev/term/$TTY >/dev/null
#RET=$?
#if [ $RET = 1 ]
if [ 1 = 1 ]
then
	STREAM=YES
else
	if [ $RET = 0 ]
	then
		STREAM=NO
	else
		# "Cannot open Device"
		echo "The port $i could not be opened.\n" >>/usr/tmp/ap.$VPID
		continue
	fi
fi
fi

if [ "$STREAM" = "NO" ]
then
	DOTTTY=$TTY



	case $TYPE in
		Terminal)
			echo "$TID:23:respawn:/etc/getty /dev/term/${TTY} $SPEED" > /etc/conf/init.d/ua_${PREFIX}${ID}
			trap '' 1 2 3 9 15
			/etc/conf/bin/idmkinit -o /usr/tmp
			cp /usr/tmp/inittab /usr/tmp/inittab$$
			chown bin /usr/tmp/inittab$$
			chgrp bin /usr/tmp/inittab$$
			chmod 444 /usr/tmp/inittab$$
			cp /usr/tmp/inittab$$ /etc/inittab
			telinit q
			rm -rf /usr/tmp/inittab$$ /usr/tmp/inittab /usr/tmp/oinittab
			echo "The port $i was setup.\n" >>/usr/tmp/ap.$VPID
			;;
	esac


else
	sacadm -l | grep ttymon3 >/dev/null 2>&1
	if [ $? = 1 ]
	then
		sacadm -a -pttymon3 -t ttymon -c "/usr/lib/saf/ttymon" -v 1
	fi
	pmadm -r -p ttymon3 -s $TTY >/dev/null 2>&1
	pmadm -a -p ttymon3 -s $TTY -i root -fu -v 1 -m " `ttyadm -d /dev/term/$TTY -l $SPEED -s /usr/bin/login -p \"login: \" ` "
	echo "The port $i was setup.\n" >>/usr/tmp/ap.$VPID

fi
done


	echo 0
	exit 0
