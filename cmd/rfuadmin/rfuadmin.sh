#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfuadmin:rfuadmin.sh	1.12.8.3"

COPYRIGHT="
#ident	\"@(#)rfuadmin:rfuadmin.sh	1.12.8.3\"
"

#if u3b2 || i386
echo "
${COPYRIGHT}

# executed by rfudaemon on request from another system.
# this is a filter that allows other systems to execute
# selected commands. This example is used by the fumount
# command when a remote system is going down.
# System administrators may expand the case list if it is
# desired to add to the remote command list.


if [ -d /var/adm/log ]
then
	LOG=/var/adm/log/rfuadmin.log
else
	LOG=/var/adm/rfuadmin.log
fi
echo \`date\` \$* >>\$LOG
case \$1 in
'fuwarn' | 'disconnect' | 'fumount')
	TYPE=\$1
	RESRC=\$2
	GRACE=\$3
	;;
'error')
	echo \$* >>\$LOG
	echo \$* >/dev/console 2>&1
	exit 1
	;;
*)
echo \"unexpected command \\\"\$*\\\"\">>\$LOG
	echo \"unexpected command for \$0 \\\"\$*\\\"\">/dev/console 2>&1
	exit 1
	;;
esac

		# RESRC is of the form domain.resource.

R=\`echo \$RESRC | sed -e 's/\\..*//'\`	# domain of the resource
D=\`/usr/sbin/dname\`				# this domain
RESRC2=\`echo \$RESRC | sed -e 's/.*\\.//'\`	# resource name only

M=\`/sbin/mount -v  |
	while read special dummy2 mountpt dummy3 dummy4 mode dummy9
	do
		if [ \"\${special}\" = \"\${RESRC}\" ]
		then
				# if the full name is in the mount table,
				# it is unique.
			echo \$mountpt \$special \$mode
			exit 0
		else
			# otherwise,
			# if the domain of this machine is the same
			# as the that of the resource, it may be
			# mounted without the domain name specified.
			# Must be careful here, cuz if the resource
			# is 'XXX', we may find 'other_domain.XXX'
			# as well as 'XXX' in the mount table.

			if [ \"\$R\" = \"\$D\" ]
			then
					# the domain is the same
				if [ \"\${RESRC2}\" = \"\${special}\" ]
				then
					echo \$mountpt \$special \$mode 
					exit 0
				fi
			fi
		fi
	done\`
if [ \"\$M\" = \"\" ]
then
	exit 0		# it's not mounted
fi
set \$M
			# \$1 is mountpoint
			# \$2 is 'domain.resource' or 'resource' 
			# \$3 is 'read/write/remote' or 'read'
case \${TYPE} in

#		The fumount(1M) warning from a host
'fuwarn')
	if [ \"\$GRACE\" != \"0\" ]
	then
	echo \"\$1\" will be disconnected from the system in \$GRACE seconds.>>\$LOG
	/usr/sbin/wall <<!
'\$1' will be disconnected from the system in \$GRACE seconds.
!
	fi
	exit 0
	;;

'disconnect' | 'fumount')
	if [ \"\$TYPE\" = \"fumount\" ]
	then
		echo \"\$1\" is being disconnected from the system NOW!>>\$LOG
		/usr/sbin/wall <<!
'\$1' is being disconnected from the system NOW!
!
	else
		echo \"\$1\" has been disconnected from the system.>>\$LOG
		/usr/sbin/wall <<!
'\$1' has been disconnected from the system.
!
	fi
	/usr/sbin/fuser -k \$2 >>\$LOG 2>&1
	# wait for the signalled processes to die
	sleep 5
	echo umount -d \$2 >>\$LOG
	/sbin/umount -d \$2
		# for automatic remount, ...
	sleep 10
	echo \$3 | grep write >/dev/null 2>&1
	if [ \$? = 0 ]
	then 
		w=\"\"
	else
		w=\"r\"
	fi
	sh /usr/sbin/rmount -d\$w \$2 \$1 &
	exit 0
	;;

esac

" >rfuadmin
#endif
