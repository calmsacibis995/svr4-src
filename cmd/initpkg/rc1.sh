#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)initpkg:rc1.sh	1.4.6.1"

if i386
then
echo "

"> rc1
fi

if u3b2 || i386
then echo "
#	\"Run Commands\" executed when the system is changing to init state 1
#

. /etc/TIMEZONE
set \`/sbin/who -r\`
if [ \$9 = \"S\" ]
then
	echo 'The system is coming up for administration.  Please wait.'
	BOOT=yes

elif [ \$7 = \"1\" ]
then
	echo 'Changing to state 1.'
	if [ -d /etc/rc1.d ]
	then
		for f in /etc/rc1.d/K*
		{
			if [ -s \${f} ]
			then
				/sbin/sh \${f} stop
			fi
		}
	fi
fi

if [ -d /etc/rc1.d ]
then
	for f in /etc/rc1.d/S*
	{
		if [ -s \${f} ]
		then
			/sbin/sh \${f} start
		fi
	}
fi

if [ \"\${BOOT}\" = \"yes\" -a \$7 = \"1\" ]
then
	echo 'The system is ready for administration.'
elif [ \$7 = \"1\" ]
then
	echo 'Change to state 1 has been completed.'
fi
" >rc1
fi
