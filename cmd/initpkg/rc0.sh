#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.




#ident	"@(#)initpkg:./rc0.sh	1.15.14.2"

if u3b2
then echo "
#	\"Run Commands\" for init state 0
#	Leaves the system in a state where it is safe to turn off the power
#	or go to firmware.
#
#	Takes an optional argument of either "off" or "firmware"
#	to specify if this is being run for "init 0" or "init 5."
#
#	In SVR4.0, inittab has been changed to no longer do the
#	uadmin to shutdown or enter firmware.  Instead, this script
#	is responsible.  By using an optional argument,
#	compatibility is maintained while still providing the needed
#	functionality to perform the uadmin call.

echo 'The system is coming down.  Please wait.'

# make sure /usr is mounted before proceeding since init scripts
# and this shell depend on things on /usr file system
/sbin/mount /usr > /dev/null 2>&1

#	The following segment is for historical purposes.
#	There should be nothing in /etc/shutdown.d.
if [ -d /etc/shutdown.d ]
then
	for f in /etc/shutdown.d/*
	{
		if [ -s \$f ]
		then
			/sbin/sh \${f}
		fi
	}
fi
#	End of historical section

if [ -d /etc/rc0.d ]
then
	for f in /etc/rc0.d/K*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f} stop
		fi
	}

#	system cleanup functions ONLY (things that end fast!)	

	for f in /etc/rc0.d/S*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f} start
		fi
	}
fi

trap \"\" 15
/usr/sbin/killall
/usr/bin/sleep 10
/sbin/umountall
echo '
The system is down.'

# check if user wants machine brought down or firmware mode
case \"\$1\" in
	off)	/sbin/uadmin 2 0
		;;

	firm*)	/sbin/uadmin 2 2
		;;
esac
" >rc0
fi


if i386
then echo "


#	\"Run Commands\" for init state 0
#	Leaves the system in a state where it is safe to turn off the power
#	or reboot.
#
#	Takes an optional argument of either off or reboot
#	to specify if this is being run for init 0 or init 5.
#
#	In SVR4.0, inittab has been changed to no longer do the
#	uadmin to shutdown or enter firmware.  Instead, this script
#	is responsible.  By using an optional argument,
#	compatibility is maintained while still providing the needed
#	functionality to perform the uadmin call.


umask 022
echo 'The system is coming down.  Please wait.'

# make sure /usr is mounted before proceeding since init scripts
# and this shell depend on things on /usr file system
/sbin/mount /usr > /dev/null 2>&1

#	The following segment is for historical purposes.
#	There should be nothing in /etc/shutdown.d.
if [ -d /etc/shutdown.d ]
then
	for f in /etc/shutdown.d/*
	{
		if [ -s \$f ]
		then
			/sbin/sh \${f}
		fi
	}
fi
#	End of historical section

if [ -d /etc/rc0.d ]
then
	for f in /etc/rc0.d/K*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f} stop
		fi
	}

#	system cleanup functions ONLY (things that end fast!)	

	for f in /etc/rc0.d/S*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f} start
		fi
	}
fi

# PC 6300+ Style Installation - execute shutdown scripts from driver packages
if [ -d /etc/idsd.d ]
then
	for f in /etc/idsd.d/*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f}
		fi
	}
fi

trap : 15
/usr/sbin/killall
/usr/bin/sleep 10


if [ -f /etc/.copy_unix ]
then
	if [ -f /etc/conf/cf.d/unix ]
	then
		mv /stand/unix /stand/unix.old 
		if [ \$? -eq 0 ]
		then
			echo Saved the old unix kernel
			rm -f /etc/.copy_unix
		else
			echo Can not save the old unix kernel.  Not enough space in /stand
		fi

		ok=0
		cp /etc/conf/cf.d/unix /stand/unix 
		if [ ! \$? -eq 0 ]
		then
			echo Failed to install the new unix kernel in /stand
			echo Retrying after removing the old unix kernel
			rm -f /stand/unix.old
			cp /etc/conf/cf.d/unix /stand/unix 
			if [ ! \$? -eq 0 ]
			then
				ok=1
				echo "Failed to install the new unix kernel in /stand"
				echo "Linking /etc/conf/cf.d/unix to /unix instead"
				rm -f /unix
				ln /etc/conf/cf.d/unix /unix
			fi
		fi
		
		if [ \$ok -eq 0 ]
		then		
			rm -f /unix
			ln -s /stand/unix /unix
			echo Installed new unix kernel 
		fi
	fi
fi

/sbin/sync;/sbin/sync;/sbin/sync
/sbin/umountall
rm -rf /tmp/*
/sbin/sync;/sbin/sync;/sbin/sync
echo '
The system is down.'

# check if user wants machine brought down or reboot
case \"\$1\" in
	off)	/sbin/uadmin 2 0
		;;

	reboot)	/sbin/uadmin 2 2
		;;
esac
" > rc0
fi
