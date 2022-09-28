#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:rc6.sh	1.3.14.2"

if u3b2 
then
echo "
#	\"Run Commands\" for init state 6
#	Do auto-configuration now if necessary.
#

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
/usr/sbin/killall  9

/sbin/ckconfig /stand/unix /stand/system /dgn/edt_data

if [ \$? = 0 ]
then
	echo \"A new unix is being built\"

	/sbin/buildsys

	if [ \$? != 0 ]
	then
		echo \"auto-configuration failed, consult a System Administrator\"
		echo \"executing /sbin/sh, to continue shutdown sequence exit shell\"
		/sbin/sh
	fi
fi

/usr/sbin/nodgmon
/sbin/umountall
echo '
The system is down.'

# check if user wants machine restarted
case \"\$1\" in
        reboot)	echo \"\\nThe system is being restarted.\"
		/sbin/uadmin 2 1
	  	;;
esac
" > rc6
fi


if i386
then
echo "

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	\"Run Commands\" for init state 6
#	Leaves the system in a state where it is safe to turn off the power 
#	or reboot.
#

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

trap \"\" 15
/usr/sbin/killall
/usr/bin/sleep 10
/usr/sbin/killall  9

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
/sbin/sync

# check if user wants machine rebooted
case \"\$1\" in
        reboot)	echo \"\\nThe system is being rebooted.\"
		/sbin/uadmin 2 1
	  	;;
esac
" > rc6
fi
